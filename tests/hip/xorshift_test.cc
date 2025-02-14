/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include "embers/status.h"
#include "embers/rand/xorshift.cuh"

using namespace embers;

int main()
{
  std::array<uint64_t, 10> rand_ints;
  std::array<uint64_t, 3> seeds = {1, 1337, 100'000};

  // Initializing with the same seed should always get the same result
  for (const auto seed : seeds) {
    for (auto &r : rand_ints) {
      rand::xorshift128p_state state;
      rand::xorshift128p_init(&state, seed);
      r = xorshift128p(&state);
    }
    if (!std::equal(rand_ints.cbegin() + 1, rand_ints.cend(), rand_ints.cbegin())) {
      throw StatusError(Status::Code::ERROR, "Failed consistency check for seed initialization");
    }
  }

  // Resetting seed should also give the same result
  for (const auto seed : seeds) {
    rand::xorshift128p_state state;
    for (auto &r : rand_ints) {
      rand::xorshift128p_init(&state, seed);
      r = xorshift128p(&state);
    }
    if (!std::equal(rand_ints.cbegin() + 1, rand_ints.cend(), rand_ints.cbegin())) {
      throw StatusError(Status::Code::ERROR, "Failed consistency check for seed reset");
    }
  }

  // Generate the same sequence on each seed
  std::array<decltype(rand_ints), 2> rand_sequences;
  for (const auto seed : seeds) {
    rand::xorshift128p_state state;
    for (auto &seq : rand_sequences) {
      rand::xorshift128p_init(&state, seed);
      for (auto &r : seq) {
        r = xorshift128p(&state);
      }
    }
    if (!std::equal(rand_sequences.cbegin() + 1, rand_sequences.cend(), rand_sequences.cbegin())) {
      throw StatusError(Status::Code::ERROR, "Failed consistency check for sequenced generation");
    }
  }

  // Oracle check
  const std::array<uint64_t, 10> oracle = {8388641,
                                           17039425,
                                           70368761226272,
                                           70368760972577,
                                           77317842217,
                                           70454912289041,
                                           648659170074339634,
                                           1243064012282800457,
                                           1170944836964264516,
                                           1155279270268194345};
  rand::xorshift128p_state state;
  rand::xorshift128p_init(&state, seeds.at(0));

  for (unsigned int i = 0; i < oracle.size(); ++i) {
    uint64_t rand_int = xorshift128p(&state);
    if (rand_int != oracle.at(i)) {
      throw StatusError(Status::Code::ERROR, "Failed oracle check");
    }
  }

  return 0;
}
