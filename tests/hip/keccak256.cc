/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <iostream>
#include <memory>

#include <hip/hip_runtime.h>

#include "embers/memory.h"
#include "embers/status.h"
#include "embers/crypto/keccak/keccak256.cuh"
#include "embers/rand/lcgparkmiller.cuh"

#include "test_helpers.h"

using namespace embers;
using namespace embers::crypto;

__global__ void fill_buffer_random(int n, uint8_t *p, size_t size)
{
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  uint32_t random;
  rand::lcg_init_seed(&random);

  if (tid < n) {
    for (size_t i = tid; i < size; i += n) {
      p[i] = rand::lcg_parkmiller(&random) & 0xff;
    }
  }
}
int main()
{
  dim3 num_blocks(3520);
  dim3 block_size(64);
  int num_hashes = num_blocks.x * block_size.x;

  int dev;
  HIP_CHECK(hipGetDevice(&dev));

  auto hashes_cpu = host::allocate_unique<hash256_t[]>(num_hashes);
  auto hashes = device::allocate_unique<hash256_t[]>(dev, num_hashes);

  fill_buffer_random<<<num_blocks, block_size>>>(int(num_blocks.x * block_size.x),
                                                 reinterpret_cast<uint8_t *>(hashes.get()),
                                                 num_hashes * sizeof(hash256_t));

  HIP_CHECK(hipGetLastError());

  HIP_CHECK(
      hipMemcpy(hashes_cpu.get(), hashes.get(), num_hashes * sizeof(hash256_t), hipMemcpyDefault));

  keccak::g_hash256<<<num_blocks, block_size>>>(hashes.get());
  HIP_CHECK(hipGetLastError());

  for (auto i = 0; i < num_hashes; i++) {
    auto hash = &hashes_cpu[i];
    keccak::hash256_single<>(hash);
  }

  HIP_CHECK(hipDeviceSynchronize());

  auto comp = host::allocate_unique<hash256_t>();
  for (auto i = 0; i < num_hashes; i++) {
    auto hash = &hashes_cpu[i];
    HIP_CHECK(hipMemcpy(comp.get(), &hashes.get()[i], sizeof(hash256_t), hipMemcpyDefault));
    for (auto index = 0; index < HASH256_H8_NUM_INDEXES; index++) {
      if (hash->h8[index] != comp->h8[index]) {
        throw StatusError(Status::Code::ERROR);
      }
    }
  }

  return 0;
}
