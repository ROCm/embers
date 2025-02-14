/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <chrono>
#include <memory>

#include <hip/hip_runtime.h>

#include "embers/memory.h"
#include "embers/primitives/nonlocking_queue.h"

#include "test_helpers.h"

__global__ void TestQueueProd(embers::NonLockingQueue<int> *q, int start, int final)
{
  auto tid = threadIdx.x + blockDim.x * blockIdx.x;
  if (!tid) {
    for (int i = start; i <= final; i++) {
      q->Enqueue(i);
    }
  }
}

__global__ void TestQueueCons(embers::NonLockingQueue<int> *q, int target)
{
  auto tid = threadIdx.x + blockDim.x * blockIdx.x;
  if (!tid) {
    int sum = 0;
    while (sum != target) {
      sum += q->Dequeue();
    }
  }
}

int main()
{
  auto num_blocks = dim3(1);
  auto dim_blocks = dim3(1);

  auto log2_size = 0;
  auto q = embers::host::make_unique<embers::NonLockingQueue<int>>(
      embers::NonLockingQueue<int>::MakeQueueContentsHost(hipHostMallocCoherent, log2_size));

  const std::pair<int, int> r0 = {1, 1337 * 3};
  const std::pair<int, int> r1 = {r0.second + 1, 1337 * 6};
  const int sum = r1.second * (r0.first + r1.second) / 2;  // arithmetic series

  std::array<hipStream_t, 2> prod;
  hipStream_t cons;

  HIP_CHECK(hipStreamCreateWithFlags(&prod.at(0), hipStreamNonBlocking));
  HIP_CHECK(hipStreamCreateWithFlags(&prod.at(1), hipStreamNonBlocking));
  HIP_CHECK(hipStreamCreateWithFlags(&cons, hipStreamNonBlocking));

  TestQueueProd<<<num_blocks, dim_blocks, 0, prod.at(0)>>>(q.get(), r0.first, r0.second);
  HIP_CHECK(hipGetLastError());
  TestQueueProd<<<num_blocks, dim_blocks, 0, prod.at(1)>>>(q.get(), r1.first, r1.second);
  HIP_CHECK(hipGetLastError());
  TestQueueCons<<<num_blocks, dim_blocks, 0, cons>>>(q.get(), sum);
  HIP_CHECK(hipGetLastError());

  HIP_CHECK(hipStreamSynchronize(cons));

  for (auto &s : prod) HIP_CHECK(hipStreamSynchronize(s));

  std::cout << "TEST PASS\n";
  return 0;
}
