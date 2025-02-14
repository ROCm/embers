/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_COUNTERS_H_
#define _EMBERS_COUNTERS_H_

#include <thread>
#include <type_traits>

#include "embers/atomic.h"
#include "embers/primitives/backoff.h"

namespace embers
{

template <MemoryScope scope_ = MemoryScope::SYSTEM,
          std::memory_order order_ = std::memory_order_seq_cst, typename SIntType_ = int,
          typename enable_ = std::enable_if_t<std::is_integral<SIntType_>::value &&
                                              std::is_signed<SIntType_>::value> >
class MonotonicCounter
{
 private:
  atomic<SIntType_, scope_> value_;

 public:
  typedef SIntType_ counter_int_type;

  __host__ MonotonicCounter();
  __host__ ~MonotonicCounter() = default;
  __host__ MonotonicCounter(MonotonicCounter &) = delete;
  __host__ MonotonicCounter &operator=(const MonotonicCounter &) = delete;
  __host__ MonotonicCounter(MonotonicCounter &&) = default;

  __host__ __device__ SIntType_ Value() const noexcept;
  __host__ __device__ void Increment(SIntType_ amount) noexcept;
  __host__ __device__ void Check(SIntType_ val) const noexcept;
  __host__ __device__ bool Appoint(SIntType_ *val, SIntType_ num) noexcept;
  __host__ __device__ void Reset() noexcept;
};

template <MemoryScope scope_, std::memory_order order_, typename SIntType_, typename enable_>
MonotonicCounter<scope_, order_, SIntType_, enable_>::MonotonicCounter()
    : value_(atomic<SIntType_>(static_cast<SIntType_>(0)))

{
  Reset();
}

template <MemoryScope scope_, std::memory_order order_, typename SIntType_, typename enable_>
__host__ __device__ void MonotonicCounter<scope_, order_, SIntType_, enable_>::Reset() noexcept
{
  value_.store(static_cast<SIntType_>(0), std::memory_order_release);
}

template <MemoryScope scope_, std::memory_order order_, typename SIntType_, typename enable_>
__host__ __device__ void MonotonicCounter<scope_, order_, SIntType_, enable_>::Increment(
    SIntType_ amount) noexcept
{
  value_.fetch_add(amount, order_);
}

template <MemoryScope scope_, std::memory_order order_, typename SIntType_, typename enable_>
__host__ __device__ SIntType_
MonotonicCounter<scope_, order_, SIntType_, enable_>::Value() const noexcept
{
  return value_.load(order_);
}

template <MemoryScope scope_, std::memory_order order_, typename SIntType_, typename enable_>
__host__ __device__ void MonotonicCounter<scope_, order_, SIntType_, enable_>::Check(
    SIntType_ level) const noexcept
{
  while (Value() < level) backoff();
}

template <MemoryScope scope_, std::memory_order order_, typename SIntType_, typename enable_>
__host__ __device__ bool MonotonicCounter<scope_, order_, SIntType_, enable_>::Appoint(
    SIntType_ *val, SIntType_ num) noexcept
{
  auto expected = value_.load(order_);
  bool success = value_.compare_exchange_strong(expected, value_.load(order_) + num, order_,
                                                order_);
  if (success) {
    *val = expected;
  }
  return success;
}

}  // namespace embers
#endif  // _EMBERS_COUNTERS_H_
