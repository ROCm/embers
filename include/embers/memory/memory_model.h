/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_MEMORY_MODEL_H_
#define _EMBERS_MEMORY_MODEL_H_

#include <atomic>
#include <exception>
#include <stdexcept>

#include <hip/hip_runtime.h>

namespace embers
{

enum class MemoryScope : int {
  SINGLE_THREAD = __HIP_MEMORY_SCOPE_SINGLETHREAD,
  WAVEFRONT = __HIP_MEMORY_SCOPE_WAVEFRONT,
  WORKGROUP = __HIP_MEMORY_SCOPE_WORKGROUP,
  AGENT = __HIP_MEMORY_SCOPE_AGENT,
  SYSTEM = __HIP_MEMORY_SCOPE_SYSTEM,
};

template <MemoryScope scope>
consteval const char *MemoryScopeToChar()
{
  return "";
}
template <>
consteval const char *MemoryScopeToChar<MemoryScope::SINGLE_THREAD>()
{
  return "wavefront";
}
template <>
consteval const char *MemoryScopeToChar<MemoryScope::WAVEFRONT>()
{
  return "wavefront";
}
template <>
consteval const char *MemoryScopeToChar<MemoryScope::WORKGROUP>()
{
  return "workgroup";
}

template <>
consteval const char *MemoryScopeToChar<MemoryScope::AGENT>()
{
  return "agent";
}

template <>
consteval const char *MemoryScopeToChar<MemoryScope::SYSTEM>()
{
  return "";
}

__host__ __device__ inline int memory_scope_as_int(MemoryScope scope)
{
  return static_cast<std::underlying_type<MemoryScope>::type>(scope);
}

__host__ __device__ inline int std_memory_order_to_int(std::memory_order order)
{
  switch (order) {
    case std::memory_order_relaxed:
      return __ATOMIC_RELAXED;
    case std::memory_order_acquire:
      return __ATOMIC_ACQUIRE;
    case std::memory_order_consume:
      return __ATOMIC_CONSUME;
    case std::memory_order_release:
      return __ATOMIC_RELEASE;
    case std::memory_order_acq_rel:
      return __ATOMIC_ACQ_REL;
    case std::memory_order_seq_cst:
      return __ATOMIC_SEQ_CST;
  }
}

}  // namespace embers

#endif  // define _EMBERS_MEMORY_MODEL_H_
