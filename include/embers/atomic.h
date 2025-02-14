/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS__ATOMIC_H_
#define _EMBERS__ATOMIC_H_

#include <atomic>
#include <type_traits>

#include <hip/hip_runtime.h>
#include "embers/memory/memory_model.h"
namespace embers
{

// must be trivially copyable
// must be copy constructible
// must be move constructible
// must be copy assignable
// must be move assignable

template <typename T, MemoryScope scope = MemoryScope::SYSTEM>
class atomic
{
 private:
  ///! @cond
  T val_;
  __host__ void _store(T desired, std::memory_order order) noexcept;
  __device__ void _store(T desired, std::memory_order order) noexcept;
  __host__ T _load(std::memory_order order) const noexcept;
  __device__ T _load(std::memory_order order) const noexcept;

  __host__ T _exchange(T desired, std::memory_order order) noexcept;
  __device__ T _exchange(T desired, std::memory_order order) noexcept;

  __host__ T _fetch_add(T arg, std::memory_order order) noexcept;
  __device__ T _fetch_add(T arg, std::memory_order order) noexcept;
  __host__ T _fetch_sub(T arg, std::memory_order order) noexcept;
  __device__ T _fetch_sub(T arg, std::memory_order order) noexcept;

  __host__ T _fetch_and(T arg, std::memory_order order) noexcept;
  __device__ T _fetch_and(T arg, std::memory_order order) noexcept;

  __host__ T _fetch_or(T arg, std::memory_order order) noexcept;
  __device__ T _fetch_or(T arg, std::memory_order order) noexcept;

  __host__ T _fetch_xor(T arg, std::memory_order order) noexcept;
  __device__ T _fetch_xor(T arg, std::memory_order order) noexcept;

  __host__ bool _compare_exchange_strong(T &expected, T desired, std::memory_order success,
                                         std::memory_order failure) noexcept;
  __device__ bool _compare_exchange_strong(T &expected, T desired, std::memory_order success,
                                           std::memory_order failure) noexcept;

  __host__ bool _compare_exchange_weak(T &expected, T desired, std::memory_order success,
                                       std::memory_order failure) noexcept;
  __device__ bool _compare_exchange_weak(T &expected, T desired, std::memory_order success,
                                         std::memory_order failure) noexcept;

  template <typename U_ = T,
            typename = std::enable_if_t<std::is_integral<U_>::value || std::is_pointer<U_>::value> >
  __host__ T _post_increment(std::memory_order order) noexcept;

  template <typename U_ = T,
            typename = std::enable_if_t<std::is_integral<U_>::value || std::is_pointer<U_>::value> >
  __device__ T _post_increment(std::memory_order order) noexcept;
  ///! @endcond
 public:
  __host__ __device__ atomic() noexcept = default;
  ;
  __host__ __device__ constexpr atomic(T desired) noexcept;
  __host__ __device__ atomic(const atomic &) = delete;

  /// atomically assigned the desired value to the atomic variable
  __host__ __device__ T operator=(T desired) noexcept;

  /// atomically replaces the current value with desired.
  __host__ __device__ void store(T desired, std::memory_order = std::memory_order_seq_cst) noexcept;

  /// atomically loads and returns the current value of the atomic variable
  __host__ __device__ T load(std::memory_order = std::memory_order_seq_cst) const noexcept;

  /// atomically loads and returns the current value of the atomic variable
  __host__ __device__ operator T() const noexcept;

  /// atomically replaces the underlying value with desired. The operation is
  /// read-modify-write.
  __host__ __device__ T exchange(T desired, std::memory_order = std::memory_order_seq_cst) noexcept;

  /// atomically compares the value with expected and if those are bitwise
  /// equal, replaces the former with desired, else loads the actual value into
  /// expected.
  __host__ __device__ bool compare_exchange_strong(
      T &expected, T desired, std::memory_order success = std::memory_order_seq_cst,
      std::memory_order failure = std::memory_order_seq_cst) noexcept;

  /// atomically compares the value with expected and if those are bitwise
  /// equal, replaces the former with desired, else loads the actual value into
  /// expected.
  __host__ __device__ bool compare_exchange_weak(
      T &expected, T desired, std::memory_order success = std::memory_order_seq_cst,
      std::memory_order failure = std::memory_order_seq_cst) noexcept;

  /// atomically replaces the curent value with the result of arithmetic
  /// addition of the value and arg. (atomic post-increment).
  __host__ __device__ T fetch_add(T arg, std::memory_order = std::memory_order_seq_cst) noexcept;

  /// atomically replaces the curent value with the result of arithmetic
  /// addition of the value and 1. (atomic post-increment).
  template <typename U_ = T,
            typename = std::enable_if_t<std::is_integral<U_>::value || std::is_pointer<U_>::value> >
  __host__ __device__ T fetch_inc(std::memory_order = std::memory_order_seq_cst) noexcept;

  /// atomically replaces the curent value with the result of arithmetic
  /// subtraction of the value and arg. (atomic post-decrement).
  __host__ __device__ T fetch_sub(T arg, std::memory_order = std::memory_order_seq_cst) noexcept;

  /// atomically replaces the curent value with the result of bitwise And of the
  /// value and arg.
  __host__ __device__ T fetch_and(T arg, std::memory_order = std::memory_order_seq_cst) noexcept;

  /// atomically replaces the curent value with the result of bitwise Or of the
  /// value and arg.
  __host__ __device__ T fetch_or(T arg, std::memory_order = std::memory_order_seq_cst) noexcept;

  /// atomically replaces the curent value with the result of bitwise Xor of the
  /// value and arg.
  __host__ __device__ T fetch_xor(T arg, std::memory_order = std::memory_order_seq_cst) noexcept;

  /// Adds with the atomic value
  __host__ __device__ T operator+=(T arg) noexcept;

  /// Subtracts with the atomic value
  __host__ __device__ T operator-=(T arg) noexcept;

  /// Performs bitwise And with the atomic value
  __host__ __device__ T operator&=(T arg) noexcept;

  /// Performs bitwise Or with the atomic value
  __host__ __device__ T operator|=(T arg) noexcept;

  /// Performs bitwise XOr with the atomic value
  __host__ __device__ T operator^=(T arg) noexcept;

  /// Performs atomic pre-increment.
  template <typename U_ = T,
            typename = std::enable_if_t<std::is_integral<U_>::value || std::is_pointer<U_>::value> >
  __host__ __device__ T operator++() noexcept;

  /// Performs atomic post-increment.
  template <typename U_ = T,
            typename = std::enable_if_t<std::is_integral<U_>::value || std::is_pointer<U_>::value> >
  __host__ __device__ T operator++(int) noexcept;

  /// Performs atomic pre-decrement.
  template <typename U_ = T,
            typename = std::enable_if_t<std::is_integral<U_>::value || std::is_pointer<U_>::value> >
  __host__ __device__ T operator--() noexcept;

  /// Performs atomic post-decrement.
  template <typename U_ = T,
            typename = std::enable_if_t<std::is_integral<U_>::value || std::is_pointer<U_>::value> >
  __host__ __device__ T operator--(int) noexcept;
};

template <typename T, MemoryScope scope>
__host__ __device__ constexpr atomic<T, scope>::atomic(T desired) noexcept : val_(desired)
{
}

template <typename T, MemoryScope scope>
__host__ __device__ atomic<T, scope>::operator T() const noexcept
{
  return load();
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::operator=(T desired) noexcept
{
  store(desired);
  return desired;
}

template <typename T, MemoryScope scope>
template <typename U_, typename>
__host__ T atomic<T, scope>::_post_increment(std::memory_order order) noexcept
{
  return fetch_add(1, order);
}

template <typename T, MemoryScope scope>
template <typename U_, typename>
__device__ T atomic<T, scope>::_post_increment(std::memory_order order) noexcept
{
  return fetch_add(1, order);
}

template <typename T, MemoryScope scope>
template <typename U_, typename>
__host__ __device__ T atomic<T, scope>::operator++() noexcept
{
  return _post_increment(std::memory_order_seq_cst) + 1;
}

template <typename T, MemoryScope scope>
template <typename U_, typename>
__host__ __device__ T atomic<T, scope>::operator++(int) noexcept
{
  return _post_increment(std::memory_order_seq_cst);
}

template <typename T, MemoryScope scope>
template <typename U_, typename>
__host__ __device__ T atomic<T, scope>::operator--() noexcept
{
  return fetch_sub(1) - 1;
}

template <typename T, MemoryScope scope>
template <typename U_, typename>
__host__ __device__ T atomic<T, scope>::operator--(int) noexcept
{
  return fetch_sub(1);
}

template <typename T, MemoryScope scope>
__host__ __device__ void atomic<T, scope>::store(T desired, std::memory_order order) noexcept
{
  _store(desired, order);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::load(std::memory_order order) const noexcept
{
  return _load(order);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::exchange(T desired, std::memory_order order) noexcept
{
  return _exchange(desired, order);
}

template <typename T, MemoryScope scope>
__host__ __device__ bool atomic<T, scope>::compare_exchange_strong(
    T &expected, T desired, std::memory_order success, std::memory_order failure) noexcept
{
  return _compare_exchange_strong(expected, desired, success, failure);
}

template <typename T, MemoryScope scope>
__host__ __device__ bool atomic<T, scope>::compare_exchange_weak(T &expected, T desired,
                                                                 std::memory_order success,
                                                                 std::memory_order failure) noexcept
{
  return _compare_exchange_weak(expected, desired, success, failure);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::fetch_add(T arg, std::memory_order order) noexcept
{
  return _fetch_add(arg, order);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::fetch_sub(T arg, std::memory_order order) noexcept
{
  return _fetch_sub(arg, order);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::fetch_and(T arg, std::memory_order order) noexcept
{
  return _fetch_and(arg, order);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::fetch_or(T arg, std::memory_order order) noexcept
{
  return _fetch_or(arg, order);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::fetch_xor(T arg, std::memory_order order) noexcept
{
  return _fetch_xor(arg, order);
}

template <typename T, MemoryScope scope>
template <typename U_, typename>
__host__ __device__ T atomic<T, scope>::fetch_inc(std::memory_order order) noexcept
{
  return _post_increment(order);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::operator+=(T arg) noexcept
{
  return fetch_add(arg);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::operator-=(T arg) noexcept
{
  return fetch_sub(arg);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::operator&=(T arg) noexcept
{
  return fetch_and(arg);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::operator|=(T arg) noexcept
{
  return fetch_or(arg);
}

template <typename T, MemoryScope scope>
__host__ __device__ T atomic<T, scope>::operator^=(T arg) noexcept
{
  return fetch_xor(arg);
}

// device
template <typename T, MemoryScope scope>
__device__ void atomic<T, scope>::_store(T desired, std::memory_order order) noexcept
{
  __hip_atomic_store(&val_, desired, std_memory_order_to_int(order),
                     static_cast<std::underlying_type<MemoryScope>::type>(scope));
};

template <typename T, MemoryScope scope>
__device__ T atomic<T, scope>::_load(std::memory_order order) const noexcept
{
  return __hip_atomic_load(&val_, std_memory_order_to_int(order),
                           static_cast<std::underlying_type<MemoryScope>::type>(scope));
}

template <typename T, MemoryScope scope>
__device__ T atomic<T, scope>::_exchange(T desired, std::memory_order order) noexcept
{
  return __hip_atomic_exchange(&val_, desired, std_memory_order_to_int(order),
                               static_cast<std::underlying_type<MemoryScope>::type>(scope));
};

template <typename T, MemoryScope scope>
__device__ bool atomic<T, scope>::_compare_exchange_strong(T &expected, T desired,
                                                           std::memory_order success,
                                                           std::memory_order failure) noexcept
{
  return __hip_atomic_compare_exchange_strong(&val_, &expected, desired,
                                              std_memory_order_to_int(success),
                                              std_memory_order_to_int(failure),
                                              static_cast<std::underlying_type<MemoryScope>::type>(
                                                  scope));
}

template <typename T, MemoryScope scope>
__device__ bool atomic<T, scope>::_compare_exchange_weak(T &expected, T desired,
                                                         std::memory_order success,
                                                         std::memory_order failure) noexcept
{
  return __hip_atomic_compare_exchange_weak(&val_, &expected, desired,
                                            std_memory_order_to_int(success),
                                            std_memory_order_to_int(failure),
                                            static_cast<std::underlying_type<MemoryScope>::type>(
                                                scope));
}

template <typename T, MemoryScope scope>
__device__ T atomic<T, scope>::_fetch_add(T arg, std::memory_order order) noexcept
{
  return __hip_atomic_fetch_add(&val_, arg, std_memory_order_to_int(order),
                                static_cast<std::underlying_type<MemoryScope>::type>(scope));
}

template <typename T, MemoryScope scope>
__device__ T atomic<T, scope>::_fetch_sub(T arg, std::memory_order order) noexcept
{
  return _fetch_add(-arg, order);
}

template <typename T, MemoryScope scope>
__device__ T atomic<T, scope>::_fetch_and(T arg, std::memory_order order) noexcept
{
  return __hip_atomic_fetch_and(&val_, arg, std_memory_order_to_int(order),
                                static_cast<std::underlying_type<MemoryScope>::type>(scope));
}

template <typename T, MemoryScope scope>
__device__ T atomic<T, scope>::_fetch_or(T arg, std::memory_order order) noexcept
{
  return __hip_atomic_fetch_or(&val_, arg, std_memory_order_to_int(order),
                               static_cast<std::underlying_type<MemoryScope>::type>(scope));
}

template <typename T, MemoryScope scope>
__device__ T atomic<T, scope>::_fetch_xor(T arg, std::memory_order order) noexcept
{
  return __hip_atomic_fetch_xor(&val_, arg, std_memory_order_to_int(order),
                                static_cast<std::underlying_type<MemoryScope>::type>(scope));
}

// host

template <typename T, MemoryScope scope>
__host__ void atomic<T, scope>::_store(T desired, std::memory_order order) noexcept
{
  return __atomic_store_n(&val_, desired, std_memory_order_to_int(order));
}

template <typename T, MemoryScope scope>
__host__ T atomic<T, scope>::_load(std::memory_order order) const noexcept
{
  return __atomic_load_n(&val_, std_memory_order_to_int(order));
}

template <typename T, MemoryScope scope>
__host__ T atomic<T, scope>::_exchange(T desired, std::memory_order order) noexcept
{
  return __atomic_exchange_n(&val_, desired, std_memory_order_to_int(order));
}

template <typename T, MemoryScope scope>
__host__ bool atomic<T, scope>::_compare_exchange_strong(T &expected, T desired,
                                                         std::memory_order success,
                                                         std::memory_order failure) noexcept
{
  return __atomic_compare_exchange_n(&val_, &expected, desired, true /*strong*/,
                                     std_memory_order_to_int(success),
                                     std_memory_order_to_int(failure));
}

template <typename T, MemoryScope scope>
__host__ bool atomic<T, scope>::_compare_exchange_weak(T &expected, T desired,
                                                       std::memory_order success,
                                                       std::memory_order failure) noexcept
{
  return __atomic_compare_exchange_n(&val_, &expected, desired, false /*weak*/,
                                     std_memory_order_to_int(success),
                                     std_memory_order_to_int(failure));
}

template <typename T, MemoryScope scope>
__host__ T atomic<T, scope>::_fetch_add(T arg, std::memory_order order) noexcept
{
  return __atomic_fetch_add(&val_, arg, std_memory_order_to_int(order));
}

template <typename T, MemoryScope scope>
__host__ T atomic<T, scope>::_fetch_sub(T arg, std::memory_order order) noexcept
{
  return __atomic_fetch_sub(&val_, arg, std_memory_order_to_int(order));
}

template <typename T, MemoryScope scope>
__host__ T atomic<T, scope>::_fetch_and(T arg, std::memory_order order) noexcept
{
  return __atomic_fetch_and(&val_, arg, std_memory_order_to_int(order));
}

template <typename T, MemoryScope scope>
__host__ T atomic<T, scope>::_fetch_or(T arg, std::memory_order order) noexcept
{
  return __atomic_fetch_or(&val_, arg, std_memory_order_to_int(order));
}

template <typename T, MemoryScope scope>
__host__ T atomic<T, scope>::_fetch_xor(T arg, std::memory_order order) noexcept
{
  return __atomic_fetch_xor(&val_, arg, std_memory_order_to_int(order));
}
}  // namespace embers
#endif  // _EMBERS__ATOMIC_H_
