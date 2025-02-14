/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_CRYPTO_HELPERS_H_
#define _EMBERS_CRYPTO_HELPERS_H_

#include <cstdint>
#include <hip/hip_runtime.h>

namespace embers
{

namespace crypto
{

constexpr int HASH256_H8_NUM_INDEXES = 4;
constexpr int HASH256_H4_NUM_INDEXES = (2 * HASH256_H8_NUM_INDEXES);
constexpr int HASH256_H2_NUM_INDEXES = (2 * HASH256_H4_NUM_INDEXES);
constexpr int HASH256_H1_NUM_INDEXES = (2 * HASH256_H2_NUM_INDEXES);

struct hash256_t {
  union {
    uint64_t h8[HASH256_H8_NUM_INDEXES];
    uint32_t h4[HASH256_H4_NUM_INDEXES];
    uint16_t h2[HASH256_H2_NUM_INDEXES];
    uint8_t h1[HASH256_H1_NUM_INDEXES];
  };
  __host__ __device__ inline bool operator==(const hash256_t &other) const
  {
    for (auto i = 0; i < HASH256_H8_NUM_INDEXES; i++) {
      if (h8[i] != other.h8[i]) {
        return false;
      }
    }
    return true;
  }
  __host__ __device__ inline bool operator!=(const hash256_t &other) const
  {
    return !(this->operator==(other));
  }
  __host__ __device__ inline hash256_t operator^(const hash256_t &other) const
  {
    hash256_t hash;
    for (auto i = 0; i < HASH256_H8_NUM_INDEXES; i++) {
      hash.h8[i] = h8[i] ^ other.h8[i];
    }
    return hash;
  }
};
}  // namespace crypto
}  // namespace embers
#endif  // _EMBERS_CRYPTO_HELPERS_H_
