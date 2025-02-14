/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_LOCKING_QUEUE_H_
#define _EMBERS_LOCKING_QUEUE_H_

#include "embers/memory.h"
#include "embers/atomic.h"

namespace embers
{

template <typename T, typename LockType>
class LockingQueue1P1C
{
  /* Locking Queue with Single Producer Single Consumer
   *
   * When agents attempt to Enqueue / Dequeue items, they will first attempt to lock-acquire
   * the lock_ using CAS.
   *
   * as CAS is a RMW operation, the cacheline will move between the agents.
   * Thus, if we can have the lock in the same CL as the consumer / producer indexes, that is ideal.
   */

 private:
  size_t num_entries_;
  size_t cons_index_;
  size_t prod_index_;
  LockType lock_;
  unique_ptr<T[]> contents_;

  __device__ __host__ size_t QIDX(size_t index);
  __device__ __host__ size_t QWRP(size_t index);

  __device__ __host__ bool QueueFull(size_t prod, size_t cons);
  __device__ __host__ bool QueueEmpty(size_t prod, size_t cons);
  __device__ __host__ void QueueIncIndex(size_t *const index);
  __device__ __host__ void QueueIncProd();
  __device__ __host__ void QueueIncCons();
  __device__ __host__ bool QueueHasSpace(size_t n);
  __device__ __host__ size_t UnprocessedEntriesCount();

 public:
  class Contents
  {
   private:
    size_t log2_size;
    unique_ptr<T[]> data;
    Contents(size_t log2_size, unique_ptr<T[]> data) : log2_size(log2_size), data(std::move(data))
    {
    }
    friend LockingQueue1P1C;
  };
  __host__ static Contents MakeQueueContents(int hip_dev, unsigned int queue_flags,
                                             size_t log2_size);
  __host__ static Contents MakeQueueContentsHost(unsigned int queue_flags, size_t log2_size);

  __host__ LockingQueue1P1C() = default;
  __host__ ~LockingQueue1P1C() = default;
  __host__ LockingQueue1P1C(const LockingQueue1P1C &) = delete;
  __host__ LockingQueue1P1C &operator=(const LockingQueue1P1C &) = delete;
  __host__ LockingQueue1P1C(LockingQueue1P1C &&) = default;
  __host__ LockingQueue1P1C(Contents contents);

  __host__ __device__ void Enqueue(T item);
  __host__ __device__ T Dequeue();
  __host__ __device__ void EnqueueMultiple(T *item, size_t n);
  __host__ __device__ void DequeueMultiple(T *item, size_t n);
  __host__ __device__ size_t EnqueueUpTo(T *item, size_t max);
  __host__ __device__ size_t DequeueUpTo(T *item, size_t max);
};
}  // namespace embers

#include "locking_queue_impl.h"
#endif  // _EMBERS_LOCKING_QUEUE_H_
