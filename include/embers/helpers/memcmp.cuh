/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_KERNEL_MEMCMP_HIPH_
#define _EMBERS_KERNEL_MEMCMP_HIPH_

#include <hip/hip_runtime.h>

#include "embers/atomic.h"
#include "embers/rand/lcgparkmiller.cuh"
namespace embers
{

template <typename T, MemoryScope scope = MemoryScope::SYSTEM>
__device__ inline void memcmp_T(T *dst, T *src, size_t num_items, atomic<int, scope> *rc)
{
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  // only n-1 threads should participate
  for (auto i = tid; i < num_items; i += (blockDim.x * gridDim.x)) {
    if (dst[i] != src[i]) {
      (*rc)++;
    }
  }
  __syncthreads();
}

template <typename T, MemoryScope scope = MemoryScope::SYSTEM>
__global__ void g_memcmp_T(T *dst, T *src, size_t num_items, atomic<int, scope> *rc)
{
  memcmp_T<T>(dst, src, num_items, rc);
}

template <typename T, MemoryScope scope = MemoryScope::SYSTEM>
__device__ inline void memcmp_nontemporal_T(T *dst, T *src, size_t num_items,
                                            atomic<int, scope> *rc)
{
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  // only n-1 threads should participate
  for (auto i = tid; i < num_items; i += (blockDim.x * gridDim.x)) {
    T temp1 = __builtin_nontemporal_load(&src[i]);
    T temp2 = __builtin_nontemporal_load(&dst[i]);
    if (temp1 != temp2) {
      (*rc)++;
    }
  }
  __syncthreads();
}

template <typename T, MemoryScope scope = MemoryScope::SYSTEM>
__global__ void g_memcmp_nontemporal_T(T *dst, T *src, size_t num_items, atomic<int, scope> *rc)
{
  memcmp_nontemporal_T<T>(dst, src, num_items, rc);
}

template <typename T, MemoryScope scope = MemoryScope::SYSTEM>
__device__ inline void memcmp_rand_temporal_T(T *dst, T *src, size_t num_items,
                                              atomic<int, scope> *rc, uint32_t seed)
{
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  // only n-1 threads should participate
  seed += tid;
  for (auto i = tid; i < num_items; i += (blockDim.x * gridDim.x)) {
    bool use_temporal = embers::rand::lcg_parkmiller(&seed) & 0x1;
    if (use_temporal) {
      T temp1 = __builtin_nontemporal_load(&src[i]);
      T temp2 = __builtin_nontemporal_load(&dst[i]);
      if (temp1 != temp2) {
        (*rc)++;
      }
    } else {
      if (dst[i] != src[i]) {
        (*rc)++;
      }
    }
  }
  __syncthreads();
}

template <typename T, MemoryScope scope = MemoryScope::SYSTEM>
__global__ void g_memcmp_rand_temporal_T(T *dst, T *src, size_t num_items, atomic<int, scope> *rc,
                                         uint32_t seed)
{
  memcmp_rand_temporal_T<T>(dst, src, num_items, rc, seed);
}

}  // namespace embers
#endif  // _EMBERS_KERNEL_MEMCMP_HIPH_
