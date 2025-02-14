/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_KERNEL_MEMCPY_HIPH_
#define _EMBERS_KERNEL_MEMCPY_HIPH_

#include <cstdint>

#include <hip/hip_runtime.h>
#include "embers/rand/lcgparkmiller.cuh"

namespace embers
{

template <typename T>
__device__ inline void memcpy_T(T *dst, const T *src, size_t num_items)
{
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  for (size_t i = tid; i < num_items; i += (blockDim.x * gridDim.x)) {
    dst[i] = src[i];
  }
  __syncthreads();
}

template <typename T>
__global__ void g_memcpy_T(T *dst, const T *src, size_t num_items)
{
  memcpy_T<T>(dst, src, num_items);
}

template <typename T>
__device__ inline void memcpy_nontemporal_T(T *dst, T *src, size_t num_items)
{
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  for (auto i = tid; i < num_items; i += (blockDim.x * gridDim.x)) {
    T temp = __builtin_nontemporal_load(&src[i]);
    __builtin_nontemporal_store(temp, &dst[i]);
  }
  __syncthreads();
}

template <typename T>
__global__ void g_memcpy_nontemporal_T(T *dst, T *src, size_t num_items)
{
  memcpy_nontemporal_T<T>(dst, src, num_items);
}

template <typename T>
__device__ inline void memcpy_specify_temporal_iters_T(T *dst, T *src, size_t num_items,
                                                       bool *use_temporal)
{
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  for (auto i = tid; i < num_items; i += (blockDim.x * gridDim.x)) {
    if (use_temporal[i]) {
      T temp = __builtin_nontemporal_load(&src[i]);
      __builtin_nontemporal_store(temp, &dst[i]);
    } else {
      dst[i] = src[i];
    }
  }
  __syncthreads();
}

template <typename T>
__global__ void g_memcpy_specify_temporal_iters_T(T *dst, T *src, size_t num_items,
                                                  bool *use_temporal)
{
  memcpy_specify_temporal_iters_T<T>(dst, src, num_items, use_temporal);
}

template <typename T>
__device__ inline void memcpy_rand_temporal_T(T *dst, T *src, size_t num_items, uint32_t seed)
{
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  seed += tid;
  for (auto i = tid; i < num_items; i += (blockDim.x * gridDim.x)) {
    bool use_temporal = rand::lcg_parkmiller(&seed) & 0x1;
    if (use_temporal) {
      T temp = __builtin_nontemporal_load(&src[i]);
      __builtin_nontemporal_store(temp, &dst[i]);
    } else {
      dst[i] = src[i];
    }
  }
  __syncthreads();
}

template <typename T>
__global__ void g_memcpy_rand_temporal_T(T *dst, T *src, size_t num_items, uint32_t seed,
                                         uint64_t *num_clocks)
{
  uint64_t start = clock64();
  memcpy_rand_temporal_T<T>(dst, src, num_items, seed);
  if (num_clocks) {
    *num_clocks = clock64() - start;
  }
}

}  // namespace embers
#endif  // _EMBERS_KERNEL_MEMCPY_HIPH_
