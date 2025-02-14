/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_XORSHIFT_CUH_
#define _EMBERS_XORSHIFT_CUH_

#include <cstdint>
#include <limits>

#include <hip/hip_runtime.h>

namespace embers
{
namespace rand
{

// @brief xorshift128p random number generator
struct xorshift128p_state {
  uint64_t x[2];
};

// @brief initialize the xorshift128p random number generator
// @param state the state of the random number generator
// @param seed non-zero seed for the random number generator
__host__ __device__ inline void xorshift128p_init(struct xorshift128p_state *state, uint64_t seed)
{
  state->x[0] = seed;
  state->x[1] = 0;
}

// @brief generate a random number
// @param state the state of the random number generator
// \return a random number
template <int a = 23, int b = 18, int c = 5>  // a/b/c are tunable
__host__ __device__ inline uint64_t xorshift128p(struct xorshift128p_state *state)
{
  uint64_t t = state->x[0];
  uint64_t const s = state->x[1];
  state->x[0] = s;
  t ^= t << a;
  t ^= t >> b;
  t ^= s ^ (s >> c);
  state->x[1] = t;
  return t + s;
}
}  // namespace rand
}  // namespace embers
#endif  //_EMBERS_XORSHIFT_CUH_
