/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_RAND_H
#define _EMBERS_RAND_H

#include <cstdint>

#include <hip/hip_runtime.h>

namespace embers
{
namespace rand
{

template <typename T>
inline __device__ __host__ T rand(uint32_t seed)
{
  static constexpr uint64_t ANSIC_LCG_MULTIPLIER = 1103515245;
  static constexpr uint64_t ANSIC_LCG_ADDEND = 12345;

  const uint64_t rval_lo = (static_cast<uint64_t>(seed) * ANSIC_LCG_MULTIPLIER +
                            ANSIC_LCG_ADDEND) >>
                           9;
  const uint64_t rval_hi = (rval_lo * ANSIC_LCG_MULTIPLIER + ANSIC_LCG_ADDEND) << 20;
  return static_cast<T>(rval_lo ^ rval_hi);
}

}  // namespace rand
}  // namespace embers

#endif
