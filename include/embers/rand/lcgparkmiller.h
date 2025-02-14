/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_LCGPARKMILLER_H_
#define _EMBERS_LCGPARKMILLER_H_

#include <cstdint>
namespace embers
{
namespace rand
{
inline uint32_t lcg_parkmiller(uint32_t *state)
{
  uint64_t product = static_cast<uint64_t>(*state * 48271);
  uint32_t x = (product & 0x7fffffff) + (product >> 31);
  x = (x & 0x7ffffffff) + (x >> 31);
  *state = x;
  return x;
}
}  // namespace rand
}  // namespace embers
#endif  // _EMBERS_LCGPARKMILLER_H_
