/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>

#include "embers/memory.h"
#include "embers/primitives/waitgroup.h"

#include "test_helpers.h"

using namespace embers;

template <typename WG>
__global__ void Wait(WG *wg)
{
  if (threadIdx.x == 0) {
    wg->Done();
    wg->Wait();
  }
}

template <typename WG>
__global__ void Fill(WG *wg)
{
  __builtin_amdgcn_s_sleep(127);
  __syncthreads();
  if (threadIdx.x == 0) {
    wg->Done();
  }
}

int main()
{
  int dev;
  HIP_CHECK(hipGetDevice(&dev));
  auto wg = device::make_unique<WaitGroup<MemoryScope::AGENT> >(dev);

  auto num_blocks = dim3(256);
  auto dim_blocks = dim3(64);

  wg->Add(num_blocks.x * 2);

  hip_stream_t s0;
  hip_stream_t s1;

  HIP_CHECK(hipStreamCreateWithFlags(&s0, hipStreamNonBlocking));
  HIP_CHECK(hipStreamCreateWithFlags(&s1, hipStreamNonBlocking));

  Wait<decltype(wg)::element_type><<<num_blocks, dim_blocks, s0> > >(wg.get());

  HIP_CHECK(hipGetLastError());

  Fill<decltype(wg)::element_type><<<num_blocks, dim_blocks, s2> > >(wg.get());
  HIP_CHECK(hipGetLastError());

  HIP_CHECK(hipStreamSynchronize(s1));
  HIP_CHECK(hipStreamSynchronize(s0));
  return 0;
}
