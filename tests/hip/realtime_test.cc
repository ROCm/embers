/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>

#include "embers/amdgpu/realtime.h"
#include "embers/memory.h"
#include "embers/status.h"
#include "test_helpers.h"

using namespace embers;
using namespace embers::amdgpu;

__global__ void TestRealtime(uint64_t *count)
{
  auto gid = blockDim.x * blockIdx.x + threadIdx.x;
  if (gid) return;

  get_realtime(count);
}

__global__ void TestRealtime2(uint64_t *count)
{
  auto gid = blockDim.x * blockIdx.x + threadIdx.x;
  if (gid) return;

  auto realtime = get_realtime();
  *count = realtime;
}
int main()
{
  auto count = host::make_unique<uint64_t>();

  TestRealtime<<<1, 64>>>(count.get());
  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());

  if (*count == 0) {
    throw StatusError(Status::Code::ERROR, "Failed `get_realtime(uint64_t* count)` test");
  }

  count.reset();
  count = host::make_unique<uint64_t>();
  TestRealtime2<<<1, 64>>>(count.get());
  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());

  if (*count == 0) {
    throw StatusError(Status::Code::ERROR, "Failed `uint64_t get_realtime()` test");
  }

  return 0;
}
