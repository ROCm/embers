/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_SPINLOCK_H_
#define _EMBERS_SPINLOCK_H_

#include <cstdint>
#include <type_traits>

#include "embers/atomic.h"

namespace embers
{

/// @brief basic spinlock which uses atomic_add
template <MemoryScope scope = MemoryScope::SYSTEM, typename Integer = uint64_t,
          typename enable = std::enable_if_t<std::is_integral<Integer>::value> >
class SpinLock
{
  /// @cond
 private:
  atomic<Integer, scope> lock_;
  /// @endcond

 public:
  SpinLock();
  ~SpinLock() = default;
  SpinLock(const SpinLock &) = delete;
  SpinLock &operator=(const SpinLock &) = delete;
  SpinLock(SpinLock &&) = delete;

  /// Blocking method to acquire the lock.
  __host__ __device__ void Acquire();

  /// Blocking method to release the lock.
  __host__ __device__ void Release();

  // Tries to acquire the lock, returns if not availble
  __host__ __device__ bool TryAcquire();
};

}  // namespace embers

#include "spinlock_impl.h"
#endif  // _EMBERS_SPINLOCK_H_
