/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_SPINLOCK_IMPL_H_
#define _EMBERS_SPINLOCK_IMPL_H_

#include <thread>

#include "embers/primitives/backoff.h"
#include "embers/primitives/spinlock.h"

namespace embers
{
template <MemoryScope scope, typename Integer, typename enable>
SpinLock<scope, Integer, enable>::SpinLock() : lock_(atomic<Integer>(static_cast<Integer>(0)))
{
}

template <MemoryScope scope, typename Integer, typename enable>
__host__ __device__ bool SpinLock<scope, Integer, enable>::TryAcquire()
{
  return (lock_.fetch_add(static_cast<Integer>(1), std::memory_order_acquire) == 0);
}

template <MemoryScope scope, typename Integer, typename enable>
__host__ __device__ void SpinLock<scope, Integer, enable>::Acquire()
{
  while (!TryAcquire()) backoff();
}

template <MemoryScope scope, typename Integer, typename enable>
__host__ __device__ void SpinLock<scope, Integer, enable>::Release()
{
  lock_.store(static_cast<Integer>(0), std::memory_order_release);
}

}  // namespace embers

#endif  // _EMBERS_SPINLOCK_IMPL_H_
