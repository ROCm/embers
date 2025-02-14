/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef __ETHASH_CRYPTO_H
#define __ETHASH_CRYPTO_H

#include <cstdint>

#include <hip/hip_runtime.h>
#include "ethash_consts.h"

namespace embers
{
namespace crypto
{
namespace ethash
{

inline uint64_t EthGetDAGSize(uint32_t EpochNum) { return (EthashDAGSizes[EpochNum]); }
inline uint64_t EthGetCacheSize(uint32_t EpochNum) { return (EthashCacheSizes[EpochNum]); }

static const uint32_t FNV_PRIME = 0x01000193U;

inline __host__ __device__ uint32_t fnv(uint32_t x, uint32_t y) { return ((x * FNV_PRIME) ^ y); }

inline __host__ __device__ uint32_t fnv_reduce(uint32_t *v)
{
  return (fnv(fnv(fnv((v)[0], (v)[1]), (v)[2]), (v)[3]));
}

typedef union {
  uint8_t h1[64];
  uint32_t h4[16];
  uint64_t h8[8];
} CacheNode;

typedef union {
  CacheNode AsNodes[2];
  uint32_t h4[32];
} DAGSlice;

bool EthCalcEpochSeedHash(void *SeedHashPtr, uint32_t EpochNum);
void EthashGenerateCache(void *CacheNodesOut, const void *SeedHash, uint64_t CacheSize);
void LightEthash(uint8_t *OutHash, uint8_t *MixHash, const uint8_t *HeaderPoWHash,
                 const void *CacheIn, const uint64_t EpochNumber, const uint64_t Nonce);

#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64ULL - (y))))

static const uint64_t KeccakF1600RndConsts[24] =
    {0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808AULL, 0x8000000080008000ULL,
     0x000000000000808BULL, 0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
     0x000000000000008AULL, 0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000AULL,
     0x000000008000808BULL, 0x800000000000008BULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
     0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800AULL, 0x800000008000000AULL,
     0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};

static __host__ __device__ inline void keccakf(void *InState)
{
  auto st = static_cast<uint64_t *>(InState);
  for (int i = 0; i < 24; ++i) {
    uint64_t bc[5], tmp;

    bc[0] = st[4] ^ st[9] ^ st[14] ^ st[19] ^ st[24] ^
            ROTL64(st[1] ^ st[6] ^ st[11] ^ st[16] ^ st[21], 1);
    bc[1] = st[0] ^ st[5] ^ st[10] ^ st[15] ^ st[20] ^
            ROTL64(st[2] ^ st[7] ^ st[12] ^ st[17] ^ st[22], 1);
    bc[2] = st[1] ^ st[6] ^ st[11] ^ st[16] ^ st[21] ^
            ROTL64(st[3] ^ st[8] ^ st[13] ^ st[18] ^ st[23], 1);
    bc[3] = st[2] ^ st[7] ^ st[12] ^ st[17] ^ st[22] ^
            ROTL64(st[4] ^ st[9] ^ st[14] ^ st[19] ^ st[24], 1);
    bc[4] = st[3] ^ st[8] ^ st[13] ^ st[18] ^ st[23] ^
            ROTL64(st[0] ^ st[5] ^ st[10] ^ st[15] ^ st[20], 1);
    st[0] ^= bc[0];

    tmp = st[1] ^ bc[1];
    st[1] = ROTL64(st[6] ^ bc[1], 44);
    st[6] = ROTL64(st[9] ^ bc[4], 20);
    st[9] = ROTL64(st[22] ^ bc[2], 61);
    st[22] = ROTL64(st[14] ^ bc[4], 39);
    st[14] = ROTL64(st[20] ^ bc[0], 18);
    st[20] = ROTL64(st[2] ^ bc[2], 62);
    st[2] = ROTL64(st[12] ^ bc[2], 43);
    st[12] = ROTL64(st[13] ^ bc[3], 25);
    st[13] = ROTL64(st[19] ^ bc[4], 8);
    st[19] = ROTL64(st[23] ^ bc[3], 56);
    st[23] = ROTL64(st[15] ^ bc[0], 41);
    st[15] = ROTL64(st[4] ^ bc[4], 27);
    st[4] = ROTL64(st[24] ^ bc[4], 14);
    st[24] = ROTL64(st[21] ^ bc[1], 2);
    st[21] = ROTL64(st[8] ^ bc[3], 55);
    st[8] = ROTL64(st[16] ^ bc[1], 45);
    st[16] = ROTL64(st[5] ^ bc[0], 36);
    st[5] = ROTL64(st[3] ^ bc[3], 28);
    st[3] = ROTL64(st[18] ^ bc[3], 21);
    st[18] = ROTL64(st[17] ^ bc[2], 15);
    st[17] = ROTL64(st[11] ^ bc[1], 10);
    st[11] = ROTL64(st[7] ^ bc[2], 6);
    st[7] = ROTL64(st[10] ^ bc[0], 3);
    st[10] = ROTL64(tmp, 1);

    bc[0] = st[0];
    bc[1] = st[1];
    st[0] ^= (~bc[1]) & st[2];
    st[1] ^= (~st[2]) & st[3];
    st[2] ^= (~st[3]) & st[4];
    st[3] ^= (~st[4]) & bc[0];
    st[4] ^= (~bc[0]) & bc[1];
    bc[0] = st[5];
    bc[1] = st[6];
    st[5] ^= (~bc[1]) & st[7];
    st[6] ^= (~st[7]) & st[8];
    st[7] ^= (~st[8]) & st[9];
    st[8] ^= (~st[9]) & bc[0];
    st[9] ^= (~bc[0]) & bc[1];
    bc[0] = st[10];
    bc[1] = st[11];
    st[10] ^= (~bc[1]) & st[12];
    st[11] ^= (~st[12]) & st[13];
    st[12] ^= (~st[13]) & st[14];
    st[13] ^= (~st[14]) & bc[0];
    st[14] ^= (~bc[0]) & bc[1];
    bc[0] = st[15];
    bc[1] = st[16];
    st[15] ^= (~bc[1]) & st[17];
    st[16] ^= (~st[17]) & st[18];
    st[17] ^= (~st[18]) & st[19];
    st[18] ^= (~st[19]) & bc[0];
    st[19] ^= (~bc[0]) & bc[1];
    bc[0] = st[20];
    bc[1] = st[21];
    st[20] ^= (~bc[1]) & st[22];
    st[21] ^= (~st[22]) & st[23];
    st[22] ^= (~st[23]) & st[24];
    st[23] ^= (~st[24]) & bc[0];
    st[24] ^= (~bc[0]) & bc[1];

    st[0] ^= KeccakF1600RndConsts[i];
  }
}

__host__ __device__ inline void SHA3_256(void *OutHashPtr, const void *InputPtr, uint32_t InputLen)
{
  uint8_t KeccakSt[200], *OutHash;
  const uint8_t *Input;

  OutHash = static_cast<uint8_t *>(OutHashPtr);
  Input = static_cast<const uint8_t *>(InputPtr);

  // Cannot use memset() in a __host__ __device__ function
  for (int i = 0; i < 25; ++i) reinterpret_cast<uint64_t *>(KeccakSt)[i] = 0x00UL;

  for (auto i = 0u; i < InputLen; ++i) KeccakSt[i] = Input[i];

  KeccakSt[InputLen] = 0x01;
  KeccakSt[135] = 0x80;

  keccakf(KeccakSt);

  for (int i = 0; i < 32; ++i) OutHash[i] = KeccakSt[i];
}

__host__ __device__ inline void SHA3_512(void *OutHashPtr, const void *InputPtr, uint32_t InputLen)
{
  uint8_t KeccakSt[200], *OutHash;
  const uint8_t *Input;

  OutHash = static_cast<uint8_t *>(OutHashPtr);
  Input = static_cast<const uint8_t *>(InputPtr);

  // Cannot use memset() in a __host__ __device__ function
  for (int i = 0; i < 25; ++i) reinterpret_cast<uint64_t *>(KeccakSt)[i] = 0x00UL;

  for (auto i = 0u; i < InputLen; ++i) KeccakSt[i] = Input[i];

  KeccakSt[InputLen] = 0x01;
  KeccakSt[71] = 0x80;

  keccakf(KeccakSt);

  for (int i = 0; i < 64; ++i) OutHash[i] = KeccakSt[i];
}

}  // namespace ethash
}  // namespace crypto
}  // namespace embers

#endif
