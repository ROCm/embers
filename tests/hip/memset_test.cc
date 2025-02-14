/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <random>

#include <hip/hip_runtime.h>

#include "embers/helpers/memset.cuh"
#include "embers/memory.h"
#include "embers/status.h"

#include "test_helpers.h"

using namespace embers;
void test_memset(int num_blocks, int block_size, size_t num_bytes)
{
  const int num_iters = 20;
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> distrib(std::numeric_limits<uint8_t>::min(),
                                                 std::numeric_limits<uint8_t>::max());
  auto src = host::allocate_unique<uint8_t[]>(num_bytes);
  auto dst = host::allocate_unique<uint8_t[]>(num_bytes);

  for (auto iter = 0; iter < num_iters; iter++) {
    int pattern = distrib(mt);
    for (auto i = 0ul; i < num_bytes; i++) {
      dst[i] = pattern;
    }

    embers::g_memset<<<num_blocks, block_size>>>(src.get(), pattern, num_bytes);
    HIP_CHECK(hipGetLastError());
    HIP_CHECK(hipDeviceSynchronize());

    if (memcmp(src.get(), dst.get(), num_bytes) != 0) {
      throw StatusError(Status::Code::ERROR,
                           std::string("Mismatch detected on iter {}") + std::to_string(iter));
    }
    pattern++;
  }
}

int main()
{
  test_memset(256, 1024, 2000);
  test_memset(1, 128, 256);
  test_memset(100, 1, 96);
  return 0;
}
