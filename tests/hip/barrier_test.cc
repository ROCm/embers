/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <stdexcept>
#include <vector>

#include <hip/hip_runtime.h>
#include "embers/memory.h"
#include "embers/primitives/barrier.h"
#include "test_helpers.h"

using namespace embers;

constexpr unsigned int compare_val = 0xdeadbeef;

template <embers::MemoryScope scope, std::memory_order order>
__global__ void TestBarrier(int total_num_blocks, int block_offset, Barrier<scope> *b,
                            unsigned int *val)
{
  auto bid = block_offset + blockIdx.x;
  if (bid == total_num_blocks - 1) {
    *val = compare_val;
  }
  b->Sync(order);
  auto actual = *val;
  if (actual != compare_val) {
    abort();
  }
}

template <embers::MemoryScope scope = embers::MemoryScope::SYSTEM,
          std::memory_order order = std::memory_order_relaxed>
void Run(int num_blocks, int block_size, int num_streams_per_device, bool test_with_cpu)
{
  int dev_count;
  HIP_CHECK(hipGetDeviceCount(&dev_count));
  if (!dev_count) {
    throw std::runtime_error("failed to find any available GPUs");
  }
  int total_num_streams = num_streams_per_device * dev_count;
  int total_num_blocks = num_blocks * total_num_streams;
  int total_num_threads = total_num_blocks * block_size;

  if (test_with_cpu) {
    total_num_threads++;
  }
  auto bar = host::make_unique_with_attributes<Barrier<scope>>(hipHostMallocCoherent,
                                                               total_num_threads);
  auto val = host::make_unique_with_attributes<unsigned int>(hipHostMallocCoherent, 0xbadc0de);

  auto streams = std::vector<hipStream_t>(total_num_streams);
  for (auto &s : streams) {
    HIP_CHECK(hipStreamCreateWithFlags(&s, hipStreamNonBlocking));
  }

  int block_offset = 0;
  for (auto i = 0; i < total_num_streams; i++) {
    TestBarrier<scope, order><<<num_blocks, block_size, 0, streams.at(i)>>>(total_num_blocks,
                                                                            block_offset, bar.get(),
                                                                            val.get());
    block_offset += num_blocks;
  }

  if (test_with_cpu) {
    std::cout << "Synchronizing on barrier from CPU" << std::endl;
    bar->Sync(order);
    auto actual = *val;
    if (actual != compare_val) {
      std::cerr << "CPU thread got wrong answer: " << actual << " expected: " << compare_val
                << "\n";
      throw std::runtime_error("MISMATCH DETECTED");
    }
  }

  for (auto i = 0; i < total_num_streams; i++) {
    std::cout << "waiting for stream: " << i << " Device: " << i / num_streams_per_device
              << " to complete" << "\n";
    HIP_CHECK(hipStreamSynchronize(streams.at(i)));
  }
}

int main()
{
  Run<embers::MemoryScope::SYSTEM>(4, 64, 4, false /*test without cpu*/);

  Run<embers::MemoryScope::SYSTEM>(4, 64, 4, true /*test with cpu*/);

  return 0;
}
