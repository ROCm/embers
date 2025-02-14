/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>

#include "embers/memory.h"
#include "embers/primitives/nonlocking_queue.h"

#include "test_helpers.h"
using namespace embers;

__global__ void TestQueueProd(NonLockingQueue<int> *q, int target)
{
  auto tid = threadIdx.x + blockDim.x * blockIdx.x;
  if (!tid) {
    for (int prod_val = 0; prod_val <= target; prod_val++) {
      q->Enqueue(prod_val);
    }
  }
}

__global__ void TestQueueCons(NonLockingQueue<int> *q, int target)
{
  auto tid = threadIdx.x + blockDim.x * blockIdx.x;
  if (!tid) {
    int cons_val = 0;
    while (cons_val != target) {
      cons_val = q->Dequeue();
    }
  }
}

int main()
{
  int target = 100;

  auto num_blocks = dim3(1);
  auto dim_blocks = dim3(1);

  auto log2_size = 0;
  auto q = host::make_unique<NonLockingQueue<int>>(
      NonLockingQueue<int>::MakeQueueContentsHost(hipHostMallocCoherent, log2_size));

  TestQueueProd<<<num_blocks, dim_blocks>>>(q.get(), target);
  HIP_CHECK(hipGetLastError());

  int val = 0;

  while (val != target) {
    val = q->Dequeue();
  }
  HIP_CHECK(hipDeviceSynchronize());

  TestQueueCons<<<num_blocks, dim_blocks>>>(q.get(), target);
  HIP_CHECK(hipGetLastError());
  for (int i = 0; i <= target; i++) {
    q->Enqueue(i);
  }
  HIP_CHECK(hipDeviceSynchronize());

  std::cout << "TEST PASS\n";
  return 0;
}
