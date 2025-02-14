/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <random>

#include <hip/hip_runtime.h>

#include "embers/memory.h"
#include "embers/status.h"
#include "embers/helpers/memcmp.cuh"
#include "embers/atomic.h"

#include "test_helpers.h"

using namespace embers;

template <typename T>
void test_memcmp(int num_blocks, int block_size, size_t num_items)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<T> distrib(std::numeric_limits<T>::min(),
                                           std::numeric_limits<T>::max());

  auto src = host::allocate_unique<T[]>(num_items);
  auto dst = host::allocate_unique<T[]>(num_items);
  auto rc = host::make_unique<atomic<int>>(0);

  do {
    for (auto i = 0ul; i < num_items; i++) {
      src[i] = distrib(mt);
      dst[i] = distrib(mt);
    }
  } while (memcmp(src.get(), dst.get(), num_items * sizeof(T)) == 0);

  g_memcmp_T<T><<<num_blocks, block_size>>>(dst.get(), src.get(), num_items, rc.get());
  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());

  if (rc == 0) {
    throw StatusError(Status::Code::ERROR, "Expected mismatch not detected");
  }
  *rc = 0;

  for (auto i = 0ul; i < num_items; i++) {
    dst[i] = src[i];
  }

  g_memcmp_T<T><<<num_blocks, block_size>>>(dst.get(), src.get(), num_items, rc.get());
  HIP_CHECK(hipGetLastError());

  if (memcmp(src.get(), dst.get(), num_items * sizeof(T)) != 0) {
    throw StatusError(Status::Code::ERROR, "Mismatch detected");
  }
  HIP_CHECK(hipDeviceSynchronize());

  if (*rc != 0) {
    throw StatusError(Status::Code::ERROR, "Unexpected mismatch detected");
  }
}

int main()
{
  test_memcmp<uint64_t>(256, 1024, 2000);
  test_memcmp<uint8_t>(1, 128, 256);
  test_memcmp<uint32_t>(100, 1, 96);
  return 0;
}
