/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_KERNEL_MEMSET_H_
#define _EMBERS_KERNEL_MEMSET_H_

#include <hip/hip_runtime.h>
#include <cstdint>

namespace embers
{
__device__ inline void memset(uint8_t *p, int val, size_t size)
{
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  for (auto i = tid; i < size; i += (blockDim.x * gridDim.x)) {
    p[i] = val;
  }
}

__global__ void g_memset(uint8_t *p, int val, size_t size) { memset(p, val, size); }
}  // namespace embers
#endif  // _EMBERS_KERNEL_MEMSET_H_
