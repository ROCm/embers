/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_RWLOCK_H_
#define _EMBERS_RWLOCK_H_

#include <cstdint>

#include "spinlock.h"

namespace embers
{

///@brief readers-writer lock
template <MemoryScope scope = MemoryScope::SYSTEM>
class RWLock
{
  /// @cond
 private:
  SpinLock<scope> r_;
  SpinLock<scope> g_;
  int b_;
  /// @endcond
 public:
  RWLock() : b_(0), r_(SpinLock<scope>()), g_(SpinLock<scope>()) {}
  ~RWLock() = default;
  RWLock(const RWLock &) = delete;
  RWLock &operator=(const RWLock &) = delete;
  RWLock(RWLock &&) = delete;

  /// Blocking method to acquire the lock for reading
  __host__ __device__ void AcquireShared()
  {
    r_.Acquire();
    b_++;
    if (b_ == 1) {
      g_.Acquire();
    }
    r_.Release();
  }

  /// Non-blocking method to acquire the lock for reading
  __host__ __device__ bool TryAcquireShared()
  {
    bool valid = r_.TryAcquire();
    if (!valid) return false;
    b_++;
    if (b_ == 1) {
      valid = g_.TryAcquire();
      if (!valid) {
        b_--;
      }
    }
    r_.Release();
    return valid;
  }

  /// method to release the shared reading lock
  __host__ __device__ void ReleaseShared()
  {
    r_.Acquire();
    b_--;
    if (b_ == 0) {
      g_.Release();
    }
    r_.Release();
  }

  /// blocking method to acquire the exclusive writing lock
  __host__ __device__ void Acquire() { g_.Acquire(); }

  /// non-blocking method to acquire the exclusive writing lock
  __host__ __device__ bool TryAcquire() { return g_.TryAcquire(); }

  /// method to release the exclusive writing lock
  __host__ __device__ void Release() { g_.Release(); }
};

}  // namespace embers

#endif  // _EMBERS_RWLOCK_H_
