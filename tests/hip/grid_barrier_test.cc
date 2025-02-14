/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <iostream>

#include "embers/memory.h"
#include "embers/primitives/grid_barrier.h"
#include "embers/atomic.h"

#include "test_helpers.h"

using namespace embers;

__global__ void KernelGridBarrier(GridBarrier *global_barrier, int iterations, int *current)
{
  for (int i = 1; i <= iterations; i++) {
    if (blockIdx.x == 0) {
      if (threadIdx.x == 0) {
        *current = blockIdx.x + threadIdx.x + i;
      }
    }
    global_barrier->Sync();
    if (*current != i) {
      abort();
    }
    global_barrier->Sync();
  }
}

void test_grid_size(bool use_host_memory, int gpu, int grid_size, int block_size, int iterations)
{
  HIP_CHECK(hipSetDevice(gpu));
  std::cout << "Running with grid size " << grid_size << std::endl;
  auto current = device::allocate_unique<int>(gpu);
  auto global_barrier = host::make_unique<GridBarrier>(
      use_host_memory ? GridBarrier::AllocateCountersHost(grid_size)
                      : GridBarrier::AllocateCounters(grid_size, gpu));
  std::cout << "Executing grid barrier" << std::endl;

  HIP_CHECK(hipMemset(current.get(), 0x0, sizeof(int)));
  KernelGridBarrier<<<grid_size, block_size>>>(global_barrier.get(), iterations, current.get());
  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());
}

int main(int argc, char **argv)
{
  int gpu;
  HIP_CHECK(hipGetDevice(&gpu));
  test_grid_size(false, gpu, 1, 1, 3);
  test_grid_size(false, gpu, 2, 1, 3);
  test_grid_size(false, gpu, 4, 1, 3);
  test_grid_size(false, gpu, 16, 1, 3);
  test_grid_size(false, gpu, 17, 1, 3);  // <-- this should fail on MI300 A0

  test_grid_size(true, gpu, 1, 1, 3);
  test_grid_size(true, gpu, 2, 1, 3);
  test_grid_size(true, gpu, 4, 1, 3);
  test_grid_size(true, gpu, 16, 1, 3);
  test_grid_size(true, gpu, 17, 1, 3);  // <-- this should fail on MI300 A0
  std::cout << "Test passed" << std::endl;
  return 0;
}
