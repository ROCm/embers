/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <cstdint>
#include <hip/hip_runtime.h>
#include "embers/atomic.h"
#include "embers/memory.h"
#include "test_helpers.h"

using namespace embers;

template <typename T, typename U, typename V>
__global__ void Test(T *a, U *c, V *d)
{
  a->fetch_add(5);
  a->fetch_sub(1);
  (*a)++;

  c->fetch_add(5);
  c->fetch_sub(1);
  (*c)++;

  d->fetch_add(5);
  d->fetch_sub(1);
  (*d)++;
}

int main()
{
  auto a = host::make_unique<atomic<unsigned int>>(0);
  auto c = host::make_unique<atomic<int>>(0);
  auto d = host::make_unique<atomic<long>>(0);

  Test<decltype(a)::element_type, decltype(c)::element_type, decltype(d)::element_type>
      <<<1, 1>>>(a.get(), c.get(), d.get());

  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());

  a->fetch_sub(1);
  a->fetch_add(3);

  std::cout << "a = " << std::to_string(++(*a)) << "\n";
  std::cout << "a = " << std::to_string(a->load()) << "\n";

  a->fetch_xor(~a->load());
  std::cout << "a = " << std::to_string(a->load()) << "\n";
  return 0;
};
