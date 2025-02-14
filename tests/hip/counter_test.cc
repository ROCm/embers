/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>
#include "embers/primitives/counters.h"
#include "embers/memory.h"

#include "test_helpers.h"

__global__ void TestCounter(embers::MonotonicCounter<> *counter, int num_loops)
{
  for (int i = 0; i < num_loops; i++) {
    counter->Increment(1);
  }
}

int main()
{
  int num_loops = 10;

  auto counter = embers::host::make_unique_with_attributes<embers::MonotonicCounter<>>(
      hipHostMallocCoherent);

  auto num_blocks = 256;
  auto dim_blocks = 64;
  TestCounter<<<num_blocks, dim_blocks>>>(counter.get(), num_loops);
  HIP_CHECK(hipGetLastError())
  counter->Increment(1);

  HIP_CHECK(hipDeviceSynchronize());

  auto val = counter->Value();
  auto expected = 1 + (num_blocks * dim_blocks * num_loops);
  if (val != expected) {
    std::cerr << "MISMATCH DETECTED: Counter value: " << counter->Value()
              << " expected: " << expected << "\n";
    return 1;
  }

  return 0;
}
