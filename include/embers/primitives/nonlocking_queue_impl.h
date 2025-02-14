/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_NONLOCKING_QUEUE_IMPL_H_
#define _EMBERS_NONLOCKING_QUEUE_IMPL_H_

#include "embers/primitives/backoff.h"
#include "nonlocking_queue.h"
namespace embers
{

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ typename NonLockingQueue<T, MonCntType, scope_>::Contents
NonLockingQueue<T, MonCntType, scope_>::MakeQueueContents(
    int hip_dev, unsigned int queue_flags, typename MonCntType::counter_int_type log2_size)
{
  return Contents(log2_size, device::make_unique_with_attributes<
                                 Entry[]>(hip_dev, queue_flags,
                                          typename MonCntType::counter_int_type(1) << log2_size));
}
template <typename T, typename MonCntType, MemoryScope scope_>
__host__ typename NonLockingQueue<T, MonCntType, scope_>::Contents
NonLockingQueue<T, MonCntType, scope_>::MakeQueueContentsHost(
    unsigned int queue_flags, typename MonCntType::counter_int_type log2_size)
{
  return Contents(log2_size,
                  host::make_unique_with_attributes<Entry[]>(queue_flags,
                                                             typename MonCntType::counter_int_type(
                                                                 1)
                                                                 << log2_size));
}

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ NonLockingQueue<T, MonCntType, scope_>::NonLockingQueue(Contents contents)
    : num_slots_(1 << contents.log2_size),
      head_(MonCntType()),
      tail_(MonCntType()),
      contents_(std::move(contents.data))
{
}

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ __device__ inline typename MonCntType::counter_int_type
NonLockingQueue<T, MonCntType, scope_>::QIDX(typename MonCntType::counter_int_type val)
{
  /* element_index = val % num_slot_s
   * Since num_slots_ is always a power of two, this can be optimized to an AND operation
   */
  return val & (num_slots_ - 1);
}

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ __device__ inline typename NonLockingQueue<T, MonCntType, scope_>::Entry *
NonLockingQueue<T, MonCntType, scope_>::GetEntry(typename MonCntType::counter_int_type index)
{
  return &contents_[std::size_t(index)];
}

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ __device__ inline bool NonLockingQueue<T, MonCntType, scope_>::Empty()
{
  return (head_.Value() - tail_.Value() == 0);
}

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ __device__ inline bool NonLockingQueue<T, MonCntType, scope_>::Full()
{
  return (head_.Value() - tail_.Value() == num_slots_);
}

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ __device__ void NonLockingQueue<T, MonCntType, scope_>::Reset()
{
  head_.Reset();
  tail_.Reset();
}

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ __device__ T NonLockingQueue<T, MonCntType, scope_>::Dequeue()
{
  while (true) {
    if (Empty()) {
      backoff();
      continue;
    }

    typename MonCntType::counter_int_type index;
    // wait until our commital has been accepted
    while (!tail_.Appoint(&index, 1)) {
    }

    Entry *e = GetEntry(QIDX(index));

    // wait for the data to be valid.
    while (!e->LockEntry(Entry::VALID)) backoff();
    e->AcquireFence();

    // read the packet
    T temp = e->data;

    e->InvalidateAndRelease();
    return temp;
  }
}

template <typename T, typename MonCntType, MemoryScope scope_>
__host__ __device__ void NonLockingQueue<T, MonCntType, scope_>::Enqueue(T item)
{
  while (true) {
    if (Full()) {
      backoff();
      continue;
    }

    typename MonCntType::counter_int_type index;
    // wait until our commital has been accepted
    while (!head_.Appoint(&index, 1)) {
    }

    Entry *e = GetEntry(QIDX(index));

    // wait for the data to be invalid.
    while (!e->LockEntry(Entry::INVALID)) backoff();

    e->data = item;
    e->SetValidAndRelease();
    return;
  }
}

}  // namespace embers
#endif  // _EMBERS_NONLOCKING_QUEUE_IMPL_H_
