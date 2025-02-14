/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>

#include "embers/helpers/memcpy.cuh"
#include "embers/memory.h"
#include "embers/status.h"

#include <random>

#include "test_helpers.h"

using namespace embers;

template <typename T>
void test_memcpy(int num_blocks, int block_size, size_t num_items)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<T> distrib(std::numeric_limits<T>::min(),
                                           std::numeric_limits<T>::max());

  auto src = host::allocate_unique<T[]>(num_items);
  auto dst = host::allocate_unique<T[]>(num_items);

  do {
    for (auto i = 0ul; i < num_items; i++) {
      src[i] = distrib(mt);
      dst[i] = distrib(mt);
    }
  } while (memcmp(src.get(), dst.get(), num_items * sizeof(T)) == 0);

  embers::g_memcpy_T<T><<<num_blocks, block_size>>>(dst.get(), src.get(), num_items);
  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());

  if (memcmp(src.get(), dst.get(), num_items * sizeof(T)) != 0) {
    throw StatusError(Status::Code::ERROR, "Mismatch detected");
  }
}

int main()
{
  test_memcpy<uint64_t>(256, 1024, 2000);
  test_memcpy<uint8_t>(1, 128, 256);
  test_memcpy<uint32_t>(100, 1, 96);
  return 0;
}
