/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef EMBERS_FILL_BUFFER_RAND_H
#define EMBERS_FILL_BUFFER_RAND_H

#include <cstdint>
#include <limits>

#include <hip/hip_runtime.h>
#include "embers/rand/xorshift.cuh"

namespace embers
{
namespace rand
{

// @brief Fill a buffer with random floating point numbers
// @param ptr Pointer to the buffer
// @param num_elems Number of elements in the buffer
// @param state Pointer to the random number generator state
// @param a Lower bound of the random numbers
// @param b Upper bound of the random numbers
template <typename T, size_t PREROUNDS = 13>
__host__ __device__ inline typename std::enable_if<std::is_floating_point<T>::value>::type
FillBufferRandom(T *ptr, size_t num_elems, xorshift128p_state *state,
                 T a = std::numeric_limits<T>::min(), T b = std::numeric_limits<T>::max())
{
#if defined(__HIP_DEVICE_COMPILE__) && __HIP_DEVICE_COMPILE__ == 1
  size_t tid = threadIdx.x + blockIdx.x * blockDim.x;
  size_t stride = gridDim.x * blockDim.x;
#else
  size_t tid = 0;
  size_t stride = 1;
#endif
  auto my_state = *state;
  my_state.x[1] = (static_cast<uint64_t>(tid) << 32) | ~static_cast<uint32_t>(tid);

  for (size_t prerounds = 0; prerounds < PREROUNDS; ++prerounds) xorshift128p(&my_state);

  for (size_t i = tid; i < num_elems; i += stride) {
    auto val = xorshift128p(&my_state);
    ptr[i] = (static_cast<T>(val) / static_cast<T>(std::numeric_limits<decltype(val)>::max())) *
                 (b - a) +
             a;
  }
}

// @brief Fill a buffer with random integer numbers
// @param ptr Pointer to the buffer
// @param num_elems Number of elements in the buffer
// @param state Pointer to the random number generator state
// @param a Lower bound of the random numbers
// @param b Upper bound of the random numbers
template <typename T, size_t PREROUNDS = 13>
__host__ __device__ inline typename std::enable_if<std::is_integral<T>::value>::type
FillBufferRandom(T *ptr, size_t num_elems, xorshift128p_state *state,
                 T a = std::numeric_limits<T>::min(), T b = std::numeric_limits<T>::max())
{
#if defined(__HIP_DEVICE_COMPILE__) && __HIP_DEVICE_COMPILE__ == 1
  size_t tid = threadIdx.x + blockIdx.x * blockDim.x;
  size_t stride = gridDim.x * blockDim.x;
#else
  size_t tid = 0;
  size_t stride = 1;
#endif
  auto my_state = *state;
  my_state.x[1] = (static_cast<uint64_t>(tid) << 32) | ~static_cast<uint32_t>(tid);

  for (size_t prerounds = 0; prerounds < PREROUNDS; ++prerounds) xorshift128p(&my_state);

  for (size_t i = tid; i < num_elems; i += stride) {
    uint64_t val = xorshift128p(&my_state);
    uint64_t tmp = static_cast<uint64_t>(b) - static_cast<uint64_t>(a);

    // Usually: `ptr[i] = a + val % tmp`
    // However; when a == b, we cannot use `tmp` since it will be zero.
    ptr[i] = tmp ? static_cast<T>(static_cast<uint64_t>(a) + val % tmp) : a;
  }
}
}  // namespace rand
}  // namespace embers

#endif  // EMBERS_FILL_BUFFER_RAND_H
