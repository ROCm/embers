/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <iostream>
#include <memory>
#include <stdint.h>

#include <hip/hip_runtime.h>
#include "embers/memory.h"
#include "embers/crypto/ethash/ethash.cuh"

#include "test_helpers.h"

using namespace embers;
using namespace embers::crypto;
using namespace embers::crypto::ethash;

// Pick an Ethash epoch, and we run Ethash with an all zero-byte
// input header hash using the dataset for the provided epoch.
// For now, we will define the epoch in the code

const uint32_t test_epoch = 256ULL;
constexpr size_t hash_input_size = 32;
constexpr size_t hash_output_size = 32;

// Supports only epochs from 0 - 2047

int main()
{
  uint8_t seed_hash_buf[hash_input_size];
  dim3 num_blocks(1 << 4);
  dim3 block_size(ETHASH_GROUP_SIZE);

  auto num_hashes = num_blocks.x * block_size.x;

  std::cout << "Generating seedhash for epoch " << test_epoch << "\n";

  // We first have to generate the seedhash for the epoch
  EthCalcEpochSeedHash(seed_hash_buf, test_epoch);

  // Now generate the smaller of the two datasets, known
  // as the "cache". Look up the size, allocate a buffer,
  // and fill it. The generation of this deliberately
  // cannot be parallelized, so it is done only on CPU.

  std::cout << "Generating cache\n";
  auto cache_size = EthGetCacheSize(test_epoch);

  auto CPUCacheBuf = host::allocate_unique<uint8_t[]>(cache_size);
  auto CPUOutHashBuf = host::allocate_unique<uint8_t[]>(num_hashes * hash_output_size);

  EthashGenerateCache(CPUCacheBuf.get(), seed_hash_buf, cache_size);

  std::cout << "Generating DAG\n";
  // The larger of the two datasets, known as the "DAG",
  // will only be generated and used on the GPU. Get the
  // size, allocate GPU memory, and run the DAG generation
  // kernel.
  uint64_t dag_size = EthGetDAGSize(test_epoch);

  // Nodes are 64 bytes each, but the lookups into the DAG
  // are 128 bytes each - so the amount of items differs
  // from the amount of nodes.
  uint32_t dag_nodes = dag_size >> 6;
  uint32_t dag_items = dag_size >> 7;

  int dev;
  HIP_CHECK(hipGetDevice(&dev));
  auto GPUDAGBuf = device::allocate_unique<uint8_t[]>(dev, dag_size);
  auto GPUCacheBuf = device::allocate_unique<uint8_t[]>(dev, cache_size);
  auto GPUInputPoWHdrHash = device::allocate_unique<uint8_t[]>(dev, hash_input_size);

  // A single DAG entry is 64 bytes, and the generation
  // kernel requires the number of entries an an argument.
  // Further, this generation kernel should only be run
  // with one thread per entry which must be generated.
  dim3 DAGGenBlockCount(dag_nodes / ETHASH_GROUP_SIZE);
  dim3 DAGGenBlockSize(ETHASH_GROUP_SIZE);

  std::cout << "Initializing GPU cache and Input PoW Header Hash\n";

  // The cache is copied from the CPU - the GPU never
  // generates it. Ensure it is copied before DAG generation.
  HIP_CHECK(hipMemcpy(GPUCacheBuf.get(), CPUCacheBuf.get(), cache_size, hipMemcpyDefault));

  // Zero out the input header hash buffer - our test
  // vector is all zero bytes.
  HIP_CHECK(hipMemset(GPUInputPoWHdrHash.get(), 0x00, hash_input_size));

  std::cout << "Dispatching GPUGenerateDag\n";
  // The generation kernel expects the number of entries
  // for the cache and DAG buffers, respectively, NOT the
  // number of bytes. Divide by 64.
  GPUGenerateDAG<<<DAGGenBlockCount, DAGGenBlockSize>>>(GPUDAGBuf.get(), GPUCacheBuf.get(),
                                                        cache_size >> 6, dag_nodes);
  HIP_CHECK(hipGetLastError());

  // Finish DAG generation and input header hash
  // zero-filling.
  auto GPUOutputHashBuf = device::allocate_unique<uint8_t[]>(dev, hash_output_size * num_hashes);

  std::cout << "Dispatching Ethash\n";
  // Note that the DAG_SIZE argument for this kernel is in 128-byte
  // chunks, as that is the size that Ethash itself accesses at a time.
  dim3 EthashBlockCount(1 << 6);
  dim3 EthashBlockSize(ETHASH_GROUP_SIZE);

  GPUEthash<<<EthashBlockCount, EthashBlockSize>>>(GPUOutputHashBuf.get(), GPUInputPoWHdrHash.get(),
                                                   GPUDAGBuf.get(), dag_items);
  HIP_CHECK(hipGetLastError());

  HIP_CHECK(hipMemcpy(CPUOutHashBuf.get(), GPUOutputHashBuf.get(), hash_output_size * num_hashes,
                      hipMemcpyDefault));

  std::cout << "Checking results\n";
  bool miscompare_detected = false;
  for (size_t i = 0; i < num_hashes; i++) {
    if (i % (num_hashes / 10) == 0) {
      std::cout << "Checking results " << i << "/" << num_hashes << "\n";
    }
    uint8_t OutHashBuf[hash_output_size], MixHashBuf[hash_input_size], HeaderBuf[hash_input_size];
    memset(HeaderBuf, 0x00, hash_input_size);
    LightEthash(OutHashBuf, MixHashBuf, HeaderBuf, CPUCacheBuf.get(), test_epoch, i);
    if (memcmp(OutHashBuf, &CPUOutHashBuf[i * hash_output_size], hash_output_size)) {
      miscompare_detected = true;
      std::cerr << "ERROR: mismatch on hash " << i << "\n";
    }
  }

  if (miscompare_detected) {
    std::cerr << "TEST FAIL\n";
    return 1;
  }
  std::cout << "TEST PASS\n";
  return 0;
}
