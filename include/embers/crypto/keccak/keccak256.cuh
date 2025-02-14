/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _KECCAK256_H_
#define _KECCAK256_H_

#include <hip/hip_runtime.h>

#include "embers/crypto/helpers.h"
#include "embers/helpers/bit_helpers.cuh"

namespace embers
{

namespace crypto
{

namespace keccak
{

template <int rounds = 24>
__host__ __device__ inline void hash256_single(hash256_t *hash)
{
  static constexpr uint64_t RC[] = {0x0000000000000001, 0x0000000000008082, 0x800000000000808A,
                                    0x8000000080008000, 0x000000000000808B, 0x0000000080000001,
                                    0x8000000080008081, 0x8000000000008009, 0x000000000000008A,
                                    0x0000000000000088, 0x0000000080008009, 0x000000008000000A,
                                    0x000000008000808B, 0x800000000000008B, 0x8000000000008089,
                                    0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
                                    0x000000000000800A, 0x800000008000000A, 0x8000000080008081,
                                    0x8000000000008080, 0x0000000080000001, 0x8000000080008008};

  uint64_t t[5];
  uint64_t u[5];
  uint64_t keccak_gpu_state[25] = {0};

  keccak_gpu_state[0] = hash->h8[0];
  keccak_gpu_state[1] = hash->h8[1];
  keccak_gpu_state[2] = hash->h8[2];
  keccak_gpu_state[3] = hash->h8[3];
  keccak_gpu_state[4] = 0x1ul;
  keccak_gpu_state[16] = 0x8000000000000000ul;

  for (auto i = 0; i < rounds; i++) {
#pragma unroll
    for (auto x = 0; x < 5; ++x) {
      t[x] = keccak_gpu_state[x] ^ keccak_gpu_state[x + 5] ^ keccak_gpu_state[x + 10] ^
             keccak_gpu_state[x + 15] ^ keccak_gpu_state[x + 20];
    }

    /* theta: d[i] = c[i+4] ^ rotl(c[i+1],1) */
    u[0] = t[4] ^ rotate<uint64_t, 1>(t[1]);
    u[1] = t[0] ^ rotate<uint64_t, 1>(t[2]);
    u[2] = t[1] ^ rotate<uint64_t, 1>(t[3]);
    u[3] = t[2] ^ rotate<uint64_t, 1>(t[4]);
    u[4] = t[3] ^ rotate<uint64_t, 1>(t[0]);

    /* theta: a[0,i], a[1,i], .. a[4,i] ^= d[i] */
    keccak_gpu_state[0] ^= u[0];
    keccak_gpu_state[5] ^= u[0];
    keccak_gpu_state[10] ^= u[0];
    keccak_gpu_state[15] ^= u[0];
    keccak_gpu_state[20] ^= u[0];
    keccak_gpu_state[1] ^= u[1];
    keccak_gpu_state[6] ^= u[1];
    keccak_gpu_state[11] ^= u[1];
    keccak_gpu_state[16] ^= u[1];
    keccak_gpu_state[21] ^= u[1];
    keccak_gpu_state[2] ^= u[2];
    keccak_gpu_state[7] ^= u[2];
    keccak_gpu_state[12] ^= u[2];
    keccak_gpu_state[17] ^= u[2];
    keccak_gpu_state[22] ^= u[2];
    keccak_gpu_state[3] ^= u[3];
    keccak_gpu_state[8] ^= u[3];
    keccak_gpu_state[13] ^= u[3];
    keccak_gpu_state[18] ^= u[3];
    keccak_gpu_state[23] ^= u[3];
    keccak_gpu_state[4] ^= u[4];
    keccak_gpu_state[9] ^= u[4];
    keccak_gpu_state[14] ^= u[4];
    keccak_gpu_state[19] ^= u[4];
    keccak_gpu_state[24] ^= u[4];

    /* rho pi: b[..] = rotl(a[..], ..) */
    t[0] = keccak_gpu_state[1];
    keccak_gpu_state[1] = rotate<uint64_t, 44>(keccak_gpu_state[6]);
    keccak_gpu_state[6] = rotate<uint64_t, 20>(keccak_gpu_state[9]);
    keccak_gpu_state[9] = rotate<uint64_t, 61>(keccak_gpu_state[22]);
    keccak_gpu_state[22] = rotate<uint64_t, 39>(keccak_gpu_state[14]);
    keccak_gpu_state[14] = rotate<uint64_t, 18>(keccak_gpu_state[20]);
    keccak_gpu_state[20] = rotate<uint64_t, 62>(keccak_gpu_state[2]);
    keccak_gpu_state[2] = rotate<uint64_t, 43>(keccak_gpu_state[12]);
    keccak_gpu_state[12] = rotate<uint64_t, 25>(keccak_gpu_state[13]);
    keccak_gpu_state[13] = rotate<uint64_t, 8>(keccak_gpu_state[19]);
    keccak_gpu_state[19] = rotate<uint64_t, 56>(keccak_gpu_state[23]);
    keccak_gpu_state[23] = rotate<uint64_t, 41>(keccak_gpu_state[15]);
    keccak_gpu_state[15] = rotate<uint64_t, 27>(keccak_gpu_state[4]);
    keccak_gpu_state[4] = rotate<uint64_t, 14>(keccak_gpu_state[24]);
    keccak_gpu_state[24] = rotate<uint64_t, 2>(keccak_gpu_state[21]);
    keccak_gpu_state[21] = rotate<uint64_t, 55>(keccak_gpu_state[8]);
    keccak_gpu_state[8] = rotate<uint64_t, 45>(keccak_gpu_state[16]);
    keccak_gpu_state[16] = rotate<uint64_t, 36>(keccak_gpu_state[5]);
    keccak_gpu_state[5] = rotate<uint64_t, 28>(keccak_gpu_state[3]);
    keccak_gpu_state[3] = rotate<uint64_t, 21>(keccak_gpu_state[18]);
    keccak_gpu_state[18] = rotate<uint64_t, 15>(keccak_gpu_state[17]);
    keccak_gpu_state[17] = rotate<uint64_t, 10>(keccak_gpu_state[11]);
    keccak_gpu_state[11] = rotate<uint64_t, 6>(keccak_gpu_state[7]);
    keccak_gpu_state[7] = rotate<uint64_t, 3>(keccak_gpu_state[10]);
    keccak_gpu_state[10] = rotate<uint64_t, 1>(t[0]);

    t[0] = keccak_gpu_state[0];
    u[0] = keccak_gpu_state[1];
    keccak_gpu_state[0] ^= (~u[0]) & keccak_gpu_state[2];
    keccak_gpu_state[1] ^= (~keccak_gpu_state[2]) & keccak_gpu_state[3];
    keccak_gpu_state[2] ^= (~keccak_gpu_state[3]) & keccak_gpu_state[4];
    keccak_gpu_state[3] ^= (~keccak_gpu_state[4]) & t[0];
    keccak_gpu_state[4] ^= (~t[0]) & u[0];
    t[0] = keccak_gpu_state[5];
    u[0] = keccak_gpu_state[6];
    keccak_gpu_state[5] ^= (~u[0]) & keccak_gpu_state[7];
    keccak_gpu_state[6] ^= (~keccak_gpu_state[7]) & keccak_gpu_state[8];
    keccak_gpu_state[7] ^= (~keccak_gpu_state[8]) & keccak_gpu_state[9];
    keccak_gpu_state[8] ^= (~keccak_gpu_state[9]) & t[0];
    keccak_gpu_state[9] ^= (~t[0]) & u[0];
    t[0] = keccak_gpu_state[10];
    u[0] = keccak_gpu_state[11];
    keccak_gpu_state[10] ^= (~u[0]) & keccak_gpu_state[12];
    keccak_gpu_state[11] ^= (~keccak_gpu_state[12]) & keccak_gpu_state[13];
    keccak_gpu_state[12] ^= (~keccak_gpu_state[13]) & keccak_gpu_state[14];
    keccak_gpu_state[13] ^= (~keccak_gpu_state[14]) & t[0];
    keccak_gpu_state[14] ^= (~t[0]) & u[0];
    t[0] = keccak_gpu_state[15];
    u[0] = keccak_gpu_state[16];
    keccak_gpu_state[15] ^= (~u[0]) & keccak_gpu_state[17];
    keccak_gpu_state[16] ^= (~keccak_gpu_state[17]) & keccak_gpu_state[18];
    keccak_gpu_state[17] ^= (~keccak_gpu_state[18]) & keccak_gpu_state[19];
    keccak_gpu_state[18] ^= (~keccak_gpu_state[19]) & t[0];
    keccak_gpu_state[19] ^= (~t[0]) & u[0];
    t[0] = keccak_gpu_state[20];
    u[0] = keccak_gpu_state[21];
    keccak_gpu_state[20] ^= (~u[0]) & keccak_gpu_state[22];
    keccak_gpu_state[21] ^= (~keccak_gpu_state[22]) & keccak_gpu_state[23];
    keccak_gpu_state[22] ^= (~keccak_gpu_state[23]) & keccak_gpu_state[24];
    keccak_gpu_state[23] ^= (~keccak_gpu_state[24]) & t[0];
    keccak_gpu_state[24] ^= (~t[0]) & u[0];

    keccak_gpu_state[0] ^= RC[i];
  }

#pragma unroll
  for (auto i = 0; i < HASH256_H8_NUM_INDEXES; i++) {
    hash->h8[i] = keccak_gpu_state[i];
  }
}

template <int rounds>
__device__ inline void hash256(hash256_t *hashes)
{
  const auto tid = threadIdx.x + blockDim.x * blockIdx.x;
  hash256_t *hash = &hashes[tid];
  hash256_single<rounds>(hash);
}

template <int rounds = 24>
__launch_bounds__(64) __global__ void g_hash256_rounds(hash256_t *hashes)
{
  hash256<rounds>(hashes);
}

__launch_bounds__(64) __global__ void g_hash256(hash256_t *hashes)
{
  static constexpr int default_num_rounds = 24;
  hash256<default_num_rounds>(hashes);
}

}  // namespace keccak
}  // namespace crypto
}  // namespace embers
#endif  // _KECCAK256_H_
