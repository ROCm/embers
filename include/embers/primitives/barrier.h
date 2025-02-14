/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_BARRIER_H_
#define _EMBERS_BARRIER_H_

#include <thread>

#include "embers/atomic.h"
#include "embers/primitives/backoff.h"

namespace embers
{

template <MemoryScope scope = MemoryScope::SYSTEM>
class Barrier
{
 private:
  const uint64_t num_participants_;
  atomic<uint64_t, scope> first_;
  atomic<uint64_t, scope> second_;

  __host__ void threadfence() const noexcept
  {
    std::atomic_thread_fence(std::memory_order_seq_cst);
  }
  __device__ void threadfence() const noexcept
  {
    if constexpr (scope == MemoryScope::AGENT) {
      __threadfence();
    } else {
      __threadfence_system();
    }
  }

 public:
  __host__ __device__ Barrier(uint64_t num_participants = 1)
      : num_participants_(num_participants),
        first_(atomic<uint64_t>(0)),
        second_(atomic<uint64_t>(0))
  {
  }

  __host__ __device__ Barrier(const Barrier &) = delete;
  __host__ ~Barrier() = default;

  __host__ __device__ void Sync(std::memory_order order = std::memory_order_relaxed) noexcept
  {
    // the incs and loads can be relaxed here because they are to the same variable (we want the inc
    // and the loads to be ordered with each other)
    first_.fetch_inc(order);
    while (first_.load(order) % num_participants_ != 0) {
      backoff();
    }

    // the threadfence with SEQ_CST ordering being in the middle ensures that Sync has SEQ_CST
    // semantics (all acquires will happen before you leave the lock, all releases will happen
    // before you leave the lock).
    threadfence();

    // because we want to re-use the barrier, we need a second lock so that "everyone has left the
    // previous lock before I let anyone move on"
    second_.fetch_inc(order);
    while (second_.load(order) % num_participants_ != 0) {
      backoff();
    }
  }
};

}  // namespace embers
#endif  // _EMBERS_BARRIER_H_
