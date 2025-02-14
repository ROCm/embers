/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_BIT_HELPERS_H_
#define _EMBERS_BIT_HELPERS_H_

#include <type_traits>

#include <hip/hip_runtime.h>

namespace embers
{
template <typename T, int num_bits>
__host__ __device__ __inline__ T rotate(T val)
{
  static constexpr int bits_in_T_minus1 = (sizeof(T) * 8) - 1;
  return (val << num_bits) | (val >> (-num_bits & bits_in_T_minus1));
}

template <typename T, typename = std::enable_if<std::is_unsigned<T>::value, bool> >
__host__ __device__ constexpr T get_bits(T val, unsigned msb, unsigned lsb) noexcept
{
  T mask = ((static_cast<T>(1) << msb) - static_cast<T>(1)) * 2 + 1;
  return (val & mask) >> lsb;
}

}  // namespace embers

#endif  // _EMBERS_BIT_HELPERS_CUH_
