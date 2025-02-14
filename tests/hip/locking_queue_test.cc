/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <cstdint>

#include <hip/hip_runtime.h>

#include "embers/memory.h"
#include "embers/primitives/locking_queue.h"
#include "embers/primitives/spinlock.h"

#include "test_helpers.h"

using Lock = embers::SpinLock<embers::MemoryScope::SYSTEM>;
using Queue = embers::LockingQueue1P1C<int, Lock>;

__global__ void TestQueueProd(Queue *q, int target)
{
  auto tid = threadIdx.x + blockDim.x * blockIdx.x;
  if (!tid) {
    for (int prod_val = 0; prod_val <= target; prod_val++) {
      q->Enqueue(prod_val);
    }
  }
}

__global__ void TestQueueCons(Queue *q, int target)
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
  int target = 128;
  int dev;
  HIP_CHECK(hipGetDevice(&dev));

  auto q = embers::host::make_unique_with_attributes<
      Queue>(hipHostMallocCoherent, Queue::MakeQueueContentsHost(hipHostMallocCoherent, 0));

  TestQueueProd<<<1, 1>>>(q.get(), target);
  HIP_CHECK(hipGetLastError());
  int val = 0;

  while (val != target) {
    val = q->Dequeue();
  }
  HIP_CHECK(hipDeviceSynchronize());

  TestQueueCons<<<1, 1>>>(q.get(), target);
  HIP_CHECK(hipGetLastError());
  for (int i = 0; i <= target; i++) {
    q->Enqueue(i);
  }
  HIP_CHECK(hipDeviceSynchronize());

  int log2target = 7;
  q = embers::host::make_unique_with_attributes<
      Queue>(hipHostMallocCoherent,
             Queue::MakeQueueContents(dev, hipDeviceMallocDefault, log2target + 1));

  TestQueueProd<<<1, 1>>>(q.get(), target);
  TestQueueCons<<<1, 1>>>(q.get(), target);

  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());

  std::cout << "test pass\n";
  return 0;
}
