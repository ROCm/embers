/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_TICKETLOCK_H_
#define _EMBERS_TICKETLOCK_H_

#include "embers/atomic.h"
#include <cstdint>

namespace embers
{

/// @brief a type of spinlock that uses "tickets" to provide a level of fairness
template <MemoryScope scope = MemoryScope::SYSTEM, size_t BackoffBase = 1>
class TicketLock
{
  /// @cond
 private:
  atomic<uint32_t, scope> next_ticket_;
  uint8_t pad[128];
  atomic<uint32_t, scope> now_serving_;

  /// @endcond

 public:
  TicketLock();
  ~TicketLock() = default;
  TicketLock(const TicketLock &) = delete;
  TicketLock &operator=(const TicketLock &) = delete;
  TicketLock(TicketLock &&) = delete;

  /// Blocking method to acquire the lock.
  __host__ __device__ void Acquire() noexcept;

  /// Blocking method to release the lock.
  __host__ __device__ void Release() noexcept;
};

}  // namespace embers
#include "ticketlock_impl.h"

#endif  // _EMBERS_TICKETLOCK_H_
