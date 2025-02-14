/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_NONLOCKING_QUEUE_H_
#define _EMBERS_NONLOCKING_QUEUE_H_

#include "counters.h"
#include "embers/memory.h"
#include "embers/atomic.h"

namespace embers
{

/*
 * NonLockingQueue replaces the spinlock and producer/consumer indexes from
 * LockingQueue1P1C with two increasingly monotonic counters.
 *
 * QUEUE_EMPTY: head_.Value() == tail_.Value()
 * QUEUE_FULL: !EMPTY() && (Index(head_.Value()) == Index(tail_.Value()))
 */

template <typename T,
          typename MonCntType = MonotonicCounter<MemoryScope::SYSTEM, std::memory_order_seq_cst>,
          MemoryScope scope_ = MemoryScope::SYSTEM>
class NonLockingQueue
{
 public:
  class Entry
  {
    enum state : int { INVALID = 0, LOCKED, VALID };
    atomic<int> flag;
    T data;

   public:
    Entry() : flag(0) {}

   private:
    friend class NonLockingQueue;

    __host__ __device__ inline bool IsValid()
    {
      return (VALID == flag.load(std::memory_order_relaxed));
    }

    __host__ inline void AcquireFence() { std::atomic_thread_fence(std::memory_order_acquire); };

    __device__ inline void AcquireFence() { flag.load(std::memory_order_acquire); };

    __host__ __device__ inline void SetValueAndRelease(enum state val)
    {
      flag.store(val, std::memory_order_release);
    }

    __host__ __device__ inline bool LockEntry(enum state expected)
    {
      return flag.compare_exchange_strong(*reinterpret_cast<int *>(&expected), LOCKED,
                                          std::memory_order_relaxed, std::memory_order_relaxed);
    }

    __host__ __device__ inline void SetValidAndRelease() { SetValueAndRelease(VALID); }

    __host__ __device__ inline void InvalidateAndRelease() { SetValueAndRelease(INVALID); }
  };

 private:
  int32_t num_slots_;
  MonCntType head_;
  MonCntType tail_;
  unique_ptr<Entry[]> contents_;

  __host__ __device__ bool Empty();
  __host__ __device__ bool Full();

  __host__ __device__ typename MonCntType::counter_int_type QIDX(
      typename MonCntType::counter_int_type val);

  __host__ __device__ Entry *GetEntry(typename MonCntType::counter_int_type index);

 public:
  class Contents
  {
   private:
    typename MonCntType::counter_int_type log2_size;
    unique_ptr<Entry[]> data;
    Contents(typename MonCntType::counter_int_type log2_size, unique_ptr<Entry[]> data)
        : log2_size(log2_size), data(std::move(data))
    {
    }
    friend class NonLockingQueue;
  };
  __host__ static Contents MakeQueueContents(int hip_dev, unsigned int queue_flags,
                                             typename MonCntType::counter_int_type log2_size);
  __host__ static Contents MakeQueueContentsHost(unsigned int queue_flags,
                                                 typename MonCntType::counter_int_type log2_size);
  __host__ NonLockingQueue() = default;
  __host__ ~NonLockingQueue() = default;
  __host__ NonLockingQueue(const NonLockingQueue &) = delete;
  __host__ NonLockingQueue &operator=(const NonLockingQueue &) = delete;
  __host__ NonLockingQueue(NonLockingQueue &&) = default;
  __host__ NonLockingQueue(Contents contents);
  __host__ __device__ void Enqueue(T item);
  __host__ __device__ T Dequeue();
  __host__ __device__ void Reset();
};
}  // namespace embers

#include "nonlocking_queue_impl.h"

#endif  // _EMBERS_NONLOCKING_QUEUE_H_
