/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_TICKET_LOCK_IMPL_H
#define _EMBERS_TICKET_LOCK_IMPL_H

#include "embers/primitives/backoff.h"
#include "embers/primitives/ticketlock.h"

namespace embers
{

template <MemoryScope scope, size_t BackoffBase>
TicketLock<scope, BackoffBase>::TicketLock()
    : next_ticket_(atomic<uint32_t>(0)), now_serving_(atomic<uint32_t>(0))
{
}

template <MemoryScope scope, size_t BackoffBase>
__host__ __device__ void TicketLock<scope, BackoffBase>::Acquire() noexcept
{
  auto ticket = next_ticket_.fetch_inc(std::memory_order_relaxed);

  while (true) {
    auto current_ticket = now_serving_.load(std::memory_order_relaxed);
    if (ticket == current_ticket) {
      break;
    }
    const auto num_before = static_cast<size_t>(ticket) - static_cast<size_t>(current_ticket);
    const size_t num_waits = num_before * BackoffBase;
    for (size_t wait = 0; wait < num_waits; wait++) {
      backoff();
    }
  }
  [[maybe_unused]] auto temp = now_serving_.load(std::memory_order_acquire);
}

template <MemoryScope scope, size_t BackoffBase>
__host__ __device__ void TicketLock<scope, BackoffBase>::Release() noexcept
{
  now_serving_.fetch_add(1, std::memory_order_release);
}

}  // namespace embers
#endif  // _EMBERS_TICKET_LOCK_IMPL_H
