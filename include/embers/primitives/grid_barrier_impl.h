/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_GRID_BARRIER_IMPL_
#define _EMBERS_GRID_BARRIER_IMPL_

#include "grid_barrier.h"
#include "embers/memory.h"
#include "embers/status.h"

namespace embers
{

unique_ptr<GridBarrier::Counter[]> GridBarrier::AllocateCounters(unsigned int num_blocks,
                                                                 int hip_dev)
{
  auto counters = device::allocate_unique<Counter[]>(hip_dev, num_blocks);
  auto err = hipMemset(counters.get(), 0x0, sizeof(decltype(counters)::element_type) * num_blocks);
  if (err != hipSuccess) {
    throw StatusError(Status::Code::ERROR, hipGetErrorString(err));
  }
  return counters;
};
unique_ptr<GridBarrier::Counter[]> GridBarrier::AllocateCountersHost(unsigned int num_blocks)
{
  auto counters = host::allocate_unique<Counter[]>(num_blocks);
  auto err = hipMemset(counters.get(), 0x0, sizeof(decltype(counters)::element_type) * num_blocks);
  if (err != hipSuccess) {
    throw StatusError(Status::Code::ERROR, hipGetErrorString(err));
  }
  return counters;
}

GridBarrier::GridBarrier() : d_sync(unique_ptr<Counter[]>()) {}
GridBarrier::GridBarrier(unique_ptr<Counter[]> counters) : d_sync(std::move(counters)) {}

__device__ __forceinline__ void GridBarrier::Sync() const
{
  // Threadfence and syncthreads to make sure global writes are visible before
  // thread-0 reports in with its sync counter
  __threadfence();
  __syncthreads();

  if (blockIdx.x == 0) {
    // Report in ourselves
    if (threadIdx.x == 0) {
      d_sync[blockIdx.x] = 1;
    }
    __syncthreads();

    // Wait for everyone else to report in
    for (uint32_t peer_block = threadIdx.x; peer_block < gridDim.x; peer_block += blockDim.x) {
      while (d_sync[peer_block].load(std::memory_order_relaxed) == 0) {
        __threadfence_block();
      }
    }
    __syncthreads();

    // Let everyone know it's safe to proceed
    for (uint32_t peer_block = threadIdx.x; peer_block < gridDim.x; peer_block += blockDim.x) {
      d_sync[peer_block] = 0;
    }
  } else {
    if (threadIdx.x == 0) {
      d_sync[blockIdx.x].fetch_add(1234, std::memory_order_relaxed);

      // Wait for acknowledgment
      while (d_sync[blockIdx.x].load(std::memory_order_relaxed) == 1234) {
        __threadfence_block();
      }
    }
    __syncthreads();
  }
}
}  // namespace embers
#endif  // _EMBERS_GRID_BARRIER_IMPL_
