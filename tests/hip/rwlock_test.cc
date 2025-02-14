/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>
#include "embers/memory.h"
#include "embers/primitives/rwlock.h"

#include "test_helpers.h"

template <typename RWLockType>
__global__ void lock_blockdim_with_critical_manager(RWLockType *locks, int num_iters,
                                                    uint32_t *counters)
{
  auto is_manager = threadIdx.x ? false : true;

  // one lock shared between all workgroups
  auto lock = &locks[0];

  // a counter for each local thread ID.
  auto my_counter = &counters[threadIdx.x];

  for (int i = 0; i < num_iters; i++) {
    if (is_manager) {
      lock->Acquire();
    }

    // wait for everyone to see that we have the lock
    __syncthreads();

    // critical section
    (*my_counter)++;

    __syncthreads();
    if (is_manager) {
      lock->Release();
    }
    // must wait for manager to release the lock
    __syncthreads();
  }
}

int main()
{
  int num_loops = 100;

  auto num_blocks = dim3(256);
  auto dim_blocks = dim3(128);

  size_t num_locks = dim_blocks.x;

  int dev;
  HIP_CHECK(hipGetDevice(&dev));
  auto locks = embers::device::allocate_unique<
      embers::RWLock<embers::MemoryScope::AGENT>[]>(dev, num_locks);
  auto counters = embers::device::allocate_unique<uint32_t[]>(dev, num_locks);
  auto counters_host = embers::host::allocate_unique<uint32_t[]>(num_locks);

  HIP_CHECK(
      hipMemset(locks.get(), 0x0, sizeof(embers::RWLock<embers::MemoryScope::AGENT>) * num_locks));
  HIP_CHECK((hipMemset(counters.get(), 0x0, sizeof(uint32_t) * num_locks)));
  HIP_CHECK((hipMemset(counters_host.get(), 0x0, sizeof(uint32_t) * num_locks)));

  lock_blockdim_with_critical_manager<embers::RWLock<embers::MemoryScope::AGENT>>
      <<<num_blocks, dim_blocks>>>(locks.get(), num_loops, counters.get());
  HIP_CHECK(hipGetLastError());

  HIP_CHECK(hipMemcpy(counters_host.get(), counters.get(), sizeof(uint32_t) * num_locks,
                      hipMemcpyDefault));

  bool mismatch = false;
  uint32_t expected = num_blocks.x * num_loops;
  for (auto i = 0; i < num_locks; i++) {
    auto val = counters_host.get()[i];
    if (val != expected) {
      std::cerr << "counters[" << i << "] = " << std::to_string(val) << " expected "
                << std::to_string(expected) << "\n";
      mismatch = true;
    }
  }
  if (mismatch) {
    std::cerr << "TEST FAIL\n";
    return 1;
  }
  std::cout << "TEST PASS\n";
  return 0;
}
