/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include "embers/amdgpu/arch.h"
#include <hip/hip_runtime.h>
#include "test_helpers.h"

using namespace embers::amdgpu;

__global__ void TestArch(void *sentinel)
{
  auto gfx9 = GFXArch(ArchFamily::GFX9);
  auto gfx940p = GFXArch(ArchFamily::GFX940Plus);
  auto gfx10p = GFXArch(ArchFamily::GFX10Plus);

  if (gfx9.Family() != ArchFamily::GFX9) {
    abort();
  }
  if (gfx940p.Family() != ArchFamily::GFX940Plus) {
    abort();
  }
  if (gfx10p.Family() != ArchFamily::GFX10Plus) {
    abort();
  }
}

int main()
{
  auto gfx9 = GFXArch(ArchFamily::GFX9);
  auto gfx940p = GFXArch(ArchFamily::GFX940Plus);
  auto gfx10p = GFXArch(ArchFamily::GFX10Plus);

  if (std::string(gfx9) != "GFX9") {
    std::cerr << "GFX9 string mismatch" << "\n";
    return -1;
  }
  if (std::string(gfx940p) != "GFX940Plus") {
    std::cerr << "GFX940Plus string mismatch" << "\n";
    return -1;
  }
  if (std::string(gfx10p) != "GFX10Plus") {
    std::cerr << "GFX10Plus string mismatch" << "\n";
    return -1;
  }

  TestArch<<<1, 64>>>(nullptr);
  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());

  return 0;
}
