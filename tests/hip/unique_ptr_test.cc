/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>
#include "embers/memory.h"

#include "test_helpers.h"

using namespace embers;

class Bar
{
 private:
  uint32_t bar_val_;

 public:
  Bar(uint32_t bar_val) : bar_val_(bar_val) {}
  __host__ __device__ uint32_t &GetRef() { return bar_val_; }
};

class Foo
{
 private:
  embers::unique_ptr<Bar> bar_;

 public:
  Foo(embers::unique_ptr<Bar> bar) : bar_(std::move(bar)) {}
  __host__ __device__ uint32_t &GetBarRef() { return bar_->GetRef(); }
};

__global__ void Test(Foo *foo)
{
  if (threadIdx.x == 0) {
    auto &bar = foo->GetBarRef();
    bar += 1;
  }
}

int main()
{
  int gpu;
  HIP_CHECK(hipGetDevice(&gpu));
  uint32_t initial_value = 5;
  auto bar = embers::device::make_unique<Bar>(gpu, initial_value);
  auto foo = embers::host::make_unique<Foo>(std::move(bar));

  Test<<<1, 1>>>(foo.get());
  HIP_CHECK(hipGetLastError());

  HIP_CHECK(hipDeviceSynchronize());

  auto &temp = foo->GetBarRef();
  if (temp != initial_value + 1) {
    std::cerr << "TEST FAIL" << "\n";
    return -1;
  }
  std::cout << "TEST PASS" << "\n";
  return 0;
}
