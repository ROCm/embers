/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_LOCKING_QUEUE_IMPL_H_
#define _EMBERS_LOCKING_QUEUE_IMPL_H_

#include "embers/memory.h"
#include "embers/primitives/backoff.h"
#include "embers/primitives/locking_queue.h"

namespace embers
{

template <typename T, typename LockType>
__host__ typename LockingQueue1P1C<T, LockType>::Contents
LockingQueue1P1C<T, LockType>::MakeQueueContents(int hip_dev, unsigned int queue_flags,
                                                 size_t log2_size)
{
  return Contents(log2_size, device::allocate_unique_with_attributes<T[]>(hip_dev, queue_flags,
                                                                          1ull << log2_size));
}
template <typename T, typename LockType>
__host__ typename LockingQueue1P1C<T, LockType>::Contents
LockingQueue1P1C<T, LockType>::MakeQueueContentsHost(unsigned int queue_flags, size_t log2_size)
{
  return Contents(log2_size,
                  host::allocate_unique_with_attributes<T[]>(queue_flags, 1ull << log2_size));
}

template <typename T, typename LockType>
LockingQueue1P1C<T, LockType>::LockingQueue1P1C(Contents contents)
    : num_entries_(1ull << contents.log2_size),
      cons_index_(0),
      prod_index_(0),
      lock_(LockType()),
      contents_(std::move(contents.data))
{
}

template <typename T, typename LockType>
__device__ __host__ size_t LockingQueue1P1C<T, LockType>::QIDX(size_t index)
{
  return ((index) & (num_entries_ - 1));
}

template <typename T, typename LockType>
__device__ __host__ size_t LockingQueue1P1C<T, LockType>::QWRP(size_t index)
{
  return ((index) & (num_entries_));
}

template <typename T, typename LockType>
__device__ __host__ bool LockingQueue1P1C<T, LockType>::QueueFull(size_t prod, size_t cons)
{
  return QIDX(prod) == QIDX(cons) && QWRP(prod) != QWRP(cons);
}

template <typename T, typename LockType>
__device__ __host__ bool LockingQueue1P1C<T, LockType>::QueueEmpty(size_t prod, size_t cons)
{
  return QIDX(prod) == QIDX(cons) && QWRP(prod) == QWRP(cons);
}

template <typename T, typename LockType>
__device__ __host__ void LockingQueue1P1C<T, LockType>::QueueIncIndex(size_t *const index)
{
  size_t val = (QWRP(*index) | QIDX(*index)) + 1;
  *index = QWRP(val) | QIDX(val);
}

template <typename T, typename LockType>
__device__ __host__ void LockingQueue1P1C<T, LockType>::QueueIncProd()
{
  QueueIncIndex(&prod_index_);
}

template <typename T, typename LockType>
__device__ __host__ void LockingQueue1P1C<T, LockType>::QueueIncCons()
{
  QueueIncIndex(&cons_index_);
}

template <typename T, typename LockType>
__device__ __host__ bool LockingQueue1P1C<T, LockType>::QueueHasSpace(size_t n)
{
  size_t space, prod, cons;
  prod = QIDX(prod_index_);
  cons = QIDX(cons_index_);

  if (QWRP(prod_index_) == QWRP(cons_index_)) {
    space = num_entries_ - (prod - cons);
  } else {
    space = cons - prod;
  }
  return space >= n;
}

template <typename T, typename LockType>
__device__ __host__ size_t LockingQueue1P1C<T, LockType>::UnprocessedEntriesCount()
{
  size_t num, prod, cons;
  prod = QIDX(prod_index_);
  cons = QIDX(cons_index_);

  if (QWRP(prod_index_) == QWRP(cons_index_)) {
    num = prod - cons;
  } else {
    num = num_entries_ + prod - cons;
  }
  return num;
}

template <typename T, typename LockType>
__host__ __device__ void LockingQueue1P1C<T, LockType>::Enqueue(T val)
{
  size_t prod;
  size_t cons;
  T *vhead = &contents_[0];

  lock_.Acquire();
  prod = prod_index_;

  while (true) {
    cons = cons_index_;
    if (!QueueFull(prod, cons)) {
      vhead[QIDX(prod)] = val;
      QueueIncProd();
      lock_.Release();
      return;
    }
    lock_.Release();
    backoff();
    lock_.Acquire();
  }
}
template <typename T, typename LockType>
__host__ __device__ void LockingQueue1P1C<T, LockType>::EnqueueMultiple(T *item, size_t n)
{
  T *vhead = &contents_[0];

  bool done = false;
  while (!done) {
    lock_.Acquire();
    bool enough_space = QueueHasSpace(n);

    if (!enough_space) {
      lock_.Release();
      continue;
    }

    for (size_t i = 0; i < n; i++) {
      vhead[QIDX(prod_index_)] = item[i];
      QueueIncProd();
    }
    done = true;
    lock_.Release();
  }
}

template <typename T, typename LockType>
__host__ __device__ size_t LockingQueue1P1C<T, LockType>::EnqueueUpTo(T *item, size_t max)
{
  lock_.Acquire();
  while (max && (!QueueHasSpace(max))) {
    max--;
  }

  if (max) {
    T *vhead = &contents_[0];
    for (size_t i = 0; i < max; i++) {
      vhead[QIDX(prod_index_)] = item[i];
      QueueIncProd();
    }
  }
  lock_.Release();
  return max;
}

template <typename T, typename LockType>
__host__ __device__ T LockingQueue1P1C<T, LockType>::Dequeue()
{
  size_t prod;
  size_t cons;

  T *vhead = &contents_[0];

  lock_.Acquire();
  cons = cons_index_;

  while (true) {
    prod = prod_index_;
    if (!QueueEmpty(prod, cons)) {
      T val = vhead[QIDX(cons)];
      QueueIncCons();
      lock_.Release();
      return val;
    }
    lock_.Release();
    backoff();
    lock_.Acquire();
  }
}

template <typename T, typename LockType>
__host__ __device__ void LockingQueue1P1C<T, LockType>::DequeueMultiple(T *item, size_t n)
{
  T *vhead = &contents_[0];

  bool done = false;
  while (!done) {
    lock_.Acquire();
    if (UnprocessedEntriesCount() < n) {
      lock_.Release();
      continue;
    }

    for (size_t i = 0; i < n; i++) {
      item[i] = vhead[QIDX(cons_index_)];
      QueueIncCons();
    }
    done = true;
    lock_.Release();
  }
}

template <typename T, typename LockType>
__host__ __device__ size_t LockingQueue1P1C<T, LockType>::DequeueUpTo(T *item, size_t max)
{
  T *vhead = &contents_[0];

  bool done = false;
  while (!done) {
    lock_.Acquire();

    auto num_entries = UnprocessedEntriesCount();
    if (num_entries == 0) {
      lock_.Release();
      continue;
    }

    auto n = (num_entries < max) ? num_entries : max;

    for (size_t i = 0; i < n; i++) {
      item[i] = vhead[QIDX(cons_index_)];
      QueueIncCons();
    }
    done = true;
    lock_.Release();
    return n;
  }
}

}  // namespace embers
#endif  // _EMBERS_LOCKING_QUEUE_IMPL_H_
