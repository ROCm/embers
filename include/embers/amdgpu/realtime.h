/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_AMDGPU_REALTIME_H_
#define _EMBERS_AMDGPU_REALTIME_H_

#include <cstdint>

namespace embers
{
namespace amdgpu
{

__device__ inline void get_realtime(uint64_t *count)
{
#if defined(__gfx1100__) || defined(__gfx1101__) || defined(__gfx1102__) || defined(__gfx1103__)
  asm volatile("s_sendmsg_rtn_b64 %0 0x83\n s_wait_kmcnt 0" : "=r"(*count)::);
#else
  asm volatile("s_memrealtime %0\n s_waitcnt lgkmcnt(0)" : "=r"(*count)::);
#endif
}

__device__ inline uint64_t get_realtime()
{
  uint64_t count = 0;
#if defined(__gfx1100__) || defined(__gfx1101__) || defined(__gfx1102__) || defined(__gfx1103__)
  asm volatile("s_sendmsg_rtn_b64 %0 0x83\n s_wait_kmcnt 0" : "=r"(count)::);
#else
  asm volatile("s_memrealtime %0\n s_waitcnt lgkmcnt(0)" : "=r"(count)::);
#endif
  return count;
}

}  // namespace amdgpu
}  // namespace embers
#endif  // _EMBERS_AMDGPU_REALTIME_H_
