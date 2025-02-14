/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_LCGPARKMILLER_IMPL_H_
#define _EMBERS_LCGPARKMILLER_IMPL_H_

#include <cstdint>

#include <hip/hip_runtime.h>
namespace embers
{
namespace rand
{

__host__ uint32_t lcg_parkmiller(uint32_t *state)
{
  uint64_t product = (uint64_t)*state * 48271;
  uint32_t x = (product & 0x7fffffff) + (product >> 31);
  x = (x & 0x7ffffffff) + (x >> 31);
  *state = x;
  return x;
}
}  // namespace rand
}  // namespace embers

#endif  // _EMBERS_LCGPARKMILLER_IMPL_H_
