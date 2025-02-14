/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>

#include "embers/memory.h"
#include "embers/rand/lcgparkmiller.cuh"
#include "embers/rand/lcgparkmiller.h"

#include "test_helpers.h"

__device__ __host__ void do_rand(uint32_t *state, int num_rand_calls)
{
  // Initialize seed
  *state = 1337;

  for (int i = 0; i < num_rand_calls; ++i) {
    embers::rand::lcg_parkmiller(state);
  }
}

__global__ void GpuRand(uint32_t *state, int num_rand_calls) { do_rand(state, num_rand_calls); }

int main()
{
  const int num_rand_calls = 1337;
  uint32_t cpu_state;
  do_rand(&cpu_state, num_rand_calls);

  auto gpu_state = embers::host::make_unique<uint32_t>(0);
  GpuRand<<<1, 1>>>(gpu_state.get(), num_rand_calls);
  HIP_CHECK(hipGetLastError());

  HIP_CHECK(hipDeviceSynchronize());

  if (*gpu_state != cpu_state) {
    std::cerr << "MISMATCH: gpu=" << std::to_string(*gpu_state)
              << " cpu=" << std::to_string(cpu_state) << "\n";
    return 1;
  }

  std::cout << "TEST PASS\n";
  return 0;
}
