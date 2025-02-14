/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_GRID_BARRIER_H_
#define _EMBERS_GRID_BARRIER_H_

#include "embers/atomic.h"
#include "embers/memory.h"

namespace embers
{

/// @brief Provides Grid-Level Synchronization of thread blocks
class GridBarrier
{
 public:
  using Counter = atomic<unsigned int, MemoryScope::AGENT>;

 private:
  // Counters in global device memory
  unique_ptr<Counter[]> d_sync;

 public:
  /// @brief Allocates and initializes counters in memory backing device hip_dev
  ///
  /// @param num_blocks  number of thread blocks in grid
  /// @param hip_dev     Device where to allocate the counters
  static unique_ptr<Counter[]> AllocateCounters(unsigned int num_blocks, int hip_dev);

  /// @brief Allocates and initializes counters in host memory
  ///
  /// @param num_blocks  number of thread blocks in grid
  static unique_ptr<Counter[]> AllocateCountersHost(unsigned int num_blocks);

  /// @brief Default constructor
  GridBarrier();

  /// @brief Constructor
  ///
  /// @param counters  Counters allocated by AllocateCounters/AllocateCountersHost

  GridBarrier(unique_ptr<Counter[]> counters);

  /// @brief Performs synchronization (blocks until all thread blocks have
  /// gotten here)
  __device__ __forceinline__ void Sync() const;
};

}  // namespace embers
#include "grid_barrier_impl.h"
#endif  // _EMBERS_GRID_BARRIER_H_
