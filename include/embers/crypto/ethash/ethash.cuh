/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _ETHASH_H_
#define _ETHASH_H_

#include <hip/hip_runtime.h>
#include "ethash_crypto.cuh"

namespace embers
{

namespace crypto
{

namespace ethash
{

// We depend on the launch bounds fixing the x block
// dimension to 64; a macro makes this more explicit.
#define ETHASH_GROUP_SIZE 32

__host__ inline CacheNode CalcDAGItem(const CacheNode *CacheInputNodes, uint32_t NodeCount,
                                      uint32_t NodeIdx)
{
  CacheNode DAGNode = CacheInputNodes[NodeIdx % NodeCount];

  DAGNode.h4[0] ^= NodeIdx;

  SHA3_512(DAGNode.h1, DAGNode.h1, sizeof(DAGNode));

  for (uint32_t i = 0; i < 256; ++i) {
    uint32_t parent_index = fnv(NodeIdx ^ i, DAGNode.h4[i % 16]) % NodeCount;
    CacheNode const *parent = CacheInputNodes + parent_index;

    for (int j = 0; j < 16; ++j) {
      DAGNode.h4[j] *= FNV_PRIME;
      DAGNode.h4[j] ^= parent->h4[j];
    }
  }

  SHA3_512(DAGNode.h1, DAGNode.h1, sizeof(DAGNode));

  return (DAGNode);
}

// OutHash & MixHash MUST have 32 bytes allocated (at least)
__host__ inline void LightEthash(uint8_t *OutHash, uint8_t *MixHash, const uint8_t *HeaderPoWHash,
                                 const void *CacheIn, const uint64_t EpochNumber,
                                 const uint64_t Nonce)
{
  uint32_t MixState[32], TmpBuf[24], NodeCount = EthGetCacheSize(EpochNumber) / sizeof(CacheNode);
  const CacheNode *Cache;
  uint64_t DagSize;

  Cache = static_cast<const CacheNode *>(CacheIn);

  // Initial hash - append nonce to header PoW hash and
  // run it through SHA3 - this becomes the initial value
  // for the mixing state buffer. The init value is used
  // later for the final hash, and is therefore saved.
  memcpy(TmpBuf, HeaderPoWHash, 32UL);
  memcpy(TmpBuf + 8UL, &Nonce, 8UL);
  SHA3_512(reinterpret_cast<uint8_t *>(TmpBuf), reinterpret_cast<uint8_t *>(TmpBuf), 40UL);

  memcpy(MixState, TmpBuf, 64UL);

  // The other half of the state is filled by simply
  // duplicating the first half of its initial value.
  memcpy(MixState + 16UL, MixState, 64UL);
  DagSize = EthGetDAGSize(EpochNumber) / (sizeof(CacheNode) << 1);

// Main mix of Ethash
#pragma unroll 1
  for (uint32_t i = 0, Init0 = MixState[0], MixValue = MixState[0]; i < 64; ++i) {
    uint32_t row = fnv(Init0 ^ i, MixValue) % DagSize;
    DAGSlice NewDAGSlice;

    NewDAGSlice.AsNodes[0] = CalcDAGItem(Cache, NodeCount, row << 1);
    NewDAGSlice.AsNodes[1] = CalcDAGItem(Cache, NodeCount, (row << 1) + 1);

    for (uint32_t col = 0; col < 32; ++col) {
      MixState[col] = fnv(MixState[col], NewDAGSlice.h4[col]);
      MixValue = col == ((i + 1) & 0x1F) ? MixState[col] : MixValue;
    }
  }

  // The reducing of the mix state directly into where
  // it will be hashed to produce the final hash. Note
  // that the initial hash is still in the first 64
  // bytes of TmpBuf - we're appending the mix hash.
  for (int i = 0; i < 8; ++i) TmpBuf[i + 16] = fnv_reduce(MixState + (i << 2));

  memcpy(MixHash, TmpBuf + 16, 32UL);

  // Hash the initial hash and the mix hash concatenated
  // to get the final proof-of-work hash that is our output.
  SHA3_256(OutHash, reinterpret_cast<uint8_t *>(TmpBuf), 96UL);
}

// Calculates the seedhash for a given Ethash epoch.
// The buffer SeedHashPtr points to MUST have at least
// 32 bytes available!
__host__ inline bool EthCalcEpochSeedHash(void *SeedHashPtr, uint32_t EpochNum)
{
  auto *SeedHash = static_cast<uint8_t *>(SeedHashPtr);

  if (EpochNum >= 2048) return (false);

  memset(SeedHash, 0x00, 32);

  for (auto Epoch = 0u; Epoch < EpochNum; ++Epoch) {
    SHA3_256(SeedHash, SeedHash, 32);
  }

  return (true);
}

// Output (CacheNodesOut) MUST have at CacheSize bytes
__host__ inline void EthashGenerateCache(void *CacheNodesOut, const void *SeedHash,
                                         uint64_t CacheSize)
{
  uint32_t const NodeCount = static_cast<uint32_t>(CacheSize / sizeof(CacheNode));
  CacheNode *CacheNodes = static_cast<CacheNode *>(CacheNodesOut);

  SHA3_512(CacheNodes[0].h1, SeedHash, 32);

  for (uint32_t i = 1; i < NodeCount; ++i) SHA3_512(CacheNodes[i].h1, CacheNodes[i - 1].h1, 64);

  for (uint32_t i = 0; i < 3; ++i) {
    for (uint32_t x = 0; x < NodeCount; ++x) {
      const uint32_t srcidx = CacheNodes[x].h4[0] % NodeCount;
      const uint32_t destidx = (x == 0) ? NodeCount - 1 : x - 1;
      CacheNode data;

      data = CacheNodes[destidx];

      for (uint32_t z = 0; z < 16; ++z) data.h4[z] ^= CacheNodes[srcidx].h4[z];

      SHA3_512(CacheNodes[x].h1, data.h1, sizeof(data));
    }
  }
}

__device__ inline void GPUEthashDevice(void *OutHashPtr, const void *HeaderPoWHashIn,
                                       const void *DAGPtr, const uint64_t DAG_SIZE)
{
  const uint32_t gid = threadIdx.x + blockDim.x * blockIdx.x;
  uint64_t InputBuf[12], HashOutBuf[12];
  const uint64_t *HeaderPoWHash;

#if 0
  uint32_t MixState[32];
  uint64_t *MyOutHash = static_cast<uint64_t*>(OutHashPtr) + (gid * 4); 
  const CacheNode *DAG = static_cast<CacheNode *>(DAGPtr);
#else
  uint32_t MixState[8];
  auto *DAG = static_cast<const DAGSlice *>(DAGPtr);
  uint64_t *MyOutHash = static_cast<uint64_t *>(OutHashPtr) + (gid * 4);
  const uint32_t hash_id = (gid & (blockDim.x - 1)) >> 2;  // threadIdx.x >> 2;
#endif

  // Block size (group size) MUST be a multiple of 4.
  if (blockDim.x & 3) abort();
  HeaderPoWHash = static_cast<const uint64_t *>(HeaderPoWHashIn);

  for (int i = 0; i < 4; ++i) InputBuf[i] = HeaderPoWHash[i];

  InputBuf[4] = static_cast<uint64_t>(gid);  // Nonce

  SHA3_512(HashOutBuf, InputBuf, 40UL);

#if 0
  for (int i = 0; i < 32; ++i)
    MixState[i] = ((uint32_t *)HashOutBuf)[i & 15];

  // Main mix
  for (int i = 0, Init0 = MixState[0], MixValue = MixState[0]; i < 64; ++i) {
    uint32_t row = fnv(Init0 ^ i, MixValue) % DAG_SIZE;
    DAGSlice Slice;
    Slice.AsNodes[0] = DAG[row << 1];
    Slice.AsNodes[1] = DAG[(row << 1) + 1];

    for (int col = 0; col < 32; ++col) {
      MixState[col] = fnv(MixState[col], Slice.h4[col]);
      MixValue = col == ((i + 1) & 0x1F) ? MixState[col] : MixValue;
    }
  }

  for (int i = 0; i < 8; ++i) {
    ((uint32_t *)HashOutBuf)[i + 16] = fnv_reduce(MixState + (i << 2));
  }

  SHA3_256(MyOutHash, HashOutBuf, 96UL);

#else
  uint32_t init0;
  const uint32_t thread_id = threadIdx.x & 3;

  for (auto tid = 0u; tid < 4; ++tid) {
    // This is tedious, but it is the best way I know how
    // without LDS or inline ASM to use v_cndmask_b32 with
    // DPP quad permute.

    uint32_t mix2[32];

    mix2[0] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[0])));
    mix2[1] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[1])));
    mix2[2] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[2])));
    mix2[3] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[3])));
    mix2[4] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[4])));
    mix2[5] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[5])));
    mix2[6] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[6])));
    mix2[7] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[7])));
    mix2[8] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[8])));
    mix2[9] = uint32_t(__builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                                    int(reinterpret_cast<uint *>(HashOutBuf)[9])));
    mix2[10] = uint32_t(
        __builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                     int(reinterpret_cast<uint *>(HashOutBuf)[10])));
    mix2[11] = uint32_t(
        __builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                     int(reinterpret_cast<uint *>(HashOutBuf)[11])));
    mix2[12] = uint32_t(
        __builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                     int(reinterpret_cast<uint *>(HashOutBuf)[12])));
    mix2[13] = uint32_t(
        __builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                     int(reinterpret_cast<uint *>(HashOutBuf)[13])));
    mix2[14] = uint32_t(
        __builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                     int(reinterpret_cast<uint *>(HashOutBuf)[14])));
    mix2[15] = uint32_t(
        __builtin_amdgcn_ds_bpermute((int((hash_id << 2u) + tid) << 2u),
                                     int(reinterpret_cast<uint *>(HashOutBuf)[15])));

    MixState[0] = ((thread_id & 1)) ? mix2[8] : mix2[0];
    MixState[1] = ((thread_id & 1)) ? mix2[9] : mix2[1];
    MixState[2] = ((thread_id & 1)) ? mix2[10] : mix2[2];
    MixState[3] = ((thread_id & 1)) ? mix2[11] : mix2[3];
    MixState[4] = ((thread_id & 1)) ? mix2[12] : mix2[4];
    MixState[5] = ((thread_id & 1)) ? mix2[13] : mix2[5];
    MixState[6] = ((thread_id & 1)) ? mix2[14] : mix2[6];
    MixState[7] = ((thread_id & 1)) ? mix2[15] : mix2[7];

    init0 = mix2[0];

    for (uint32_t a = 0; a < 64; a += 8) {
      const uint32_t LaneIdx = (hash_id << 2) + ((a >> 3) & 3);
      for (auto x = 0u; x < 8; ++x) {
        uint32_t tmp = fnv(init0 ^ (a + x), MixState[x]) % DAG_SIZE;
        tmp = uint32_t(__builtin_amdgcn_ds_bpermute(int(LaneIdx << 2u), int(tmp)));

        for (auto i = 0u; i < 8; ++i)
          MixState[i] = fnv(MixState[i], DAG[tmp].h4[(thread_id << 3) + i]);
      }
    }

    uint32_t ReducedValLo, ReducedValHi;

    ReducedValLo = fnv_reduce(MixState);
    ReducedValHi = fnv_reduce(MixState + 4);

    uint32_t vals[8];

    vals[0] = uint32_t(
        __builtin_amdgcn_ds_bpermute(int(((hash_id << 2) + 0) << 2), int(ReducedValLo)));
    vals[1] = uint32_t(
        __builtin_amdgcn_ds_bpermute(int(((hash_id << 2) + 0) << 2), int(ReducedValHi)));

    vals[2] = uint32_t(
        __builtin_amdgcn_ds_bpermute(int(((hash_id << 2) + 1) << 2), int(ReducedValLo)));
    vals[3] = uint32_t(
        __builtin_amdgcn_ds_bpermute(int(((hash_id << 2) + 1) << 2), int(ReducedValHi)));

    vals[4] = uint32_t(
        __builtin_amdgcn_ds_bpermute(int(((hash_id << 2) + 2) << 2), int(ReducedValLo)));
    vals[5] = uint32_t(
        __builtin_amdgcn_ds_bpermute(int(((hash_id << 2) + 2) << 2), int(ReducedValHi)));

    vals[6] = uint32_t(
        __builtin_amdgcn_ds_bpermute(int(((hash_id << 2) + 3) << 2), int(ReducedValLo)));
    vals[7] = uint32_t(
        __builtin_amdgcn_ds_bpermute(int(((hash_id << 2) + 3) << 2), int(ReducedValHi)));

    if (tid == thread_id) {
      for (int i = 0; i < 4; ++i) InputBuf[8 + i] = reinterpret_cast<uint64_t *>(vals)[i];
    }
  }

  // Hash the initial hash and the mix hash concatenated
  // to get the final proof-of-work hash that is our output.
  for (int i = 0; i < 8; ++i) InputBuf[i] = HashOutBuf[i];

  SHA3_256(HashOutBuf, InputBuf, sizeof(uint64_t) * 12);

  for (int i = 0; i < 4; ++i) MyOutHash[i] = HashOutBuf[i];

#endif
}

__launch_bounds__(ETHASH_GROUP_SIZE) __global__
    void GPUEthash(void *OutHashPtr, const void *HeaderPoWHashIn, const void *DAGPtr,
                   const uint64_t DAG_SIZE)
{
  GPUEthashDevice(OutHashPtr, HeaderPoWHashIn, DAGPtr, DAG_SIZE);
}

// This should be launched with ONLY the amount
// of threads required to generate the full DAG;
// that is, it should be launched with the number
// of DAG nodes for total threads.
__device__ inline void GPUGenerateDAGDevice(void *DAGPtr, const void *CachePtr, uint32_t LIGHT_SIZE,
                                            uint64_t DAG_SIZE)
{
  const uint32_t gid = threadIdx.x + blockDim.x * blockIdx.x;
  auto Cache = static_cast<const CacheNode *>(CachePtr);
  auto DAG = static_cast<CacheNode *>(DAGPtr);
  uint32_t NodeIdx = gid;

  if (NodeIdx > DAG_SIZE) return;

  CacheNode DAGNode = Cache[NodeIdx % LIGHT_SIZE];

  DAGNode.h4[0] ^= NodeIdx;

  SHA3_512(DAGNode.h1, DAGNode.h1, 64UL);

  for (uint32_t i = 0; i < 256; ++i) {
    uint ParentIdx = fnv(NodeIdx ^ i, DAGNode.h4[i & 15]) % LIGHT_SIZE;
    const CacheNode *ParentNode = Cache + ParentIdx;

    for (uint x = 0; x < 16; ++x) {
      DAGNode.h4[x] *= (FNV_PRIME);
      DAGNode.h4[x] ^= ParentNode->h4[x];
    }
  }

  SHA3_512(DAGNode.h1, DAGNode.h1, 64UL);
  DAG[NodeIdx] = DAGNode;
}

__launch_bounds__(ETHASH_GROUP_SIZE) __global__
    void GPUGenerateDAG(void *DAGPtr, const void *CachePtr, uint32_t LIGHT_SIZE, uint64_t DAG_SIZE)
{
  GPUGenerateDAGDevice(DAGPtr, CachePtr, LIGHT_SIZE, DAG_SIZE);
}

}  // namespace ethash
}  // namespace crypto
}  // namespace embers
#endif  // _ETHASH_H_
