/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>

#include "embers/amdgpu/hwid.h"
#include "embers/memory.h"
#include "embers/status.h"
#include "test_helpers.h"

using namespace embers;
using namespace embers::amdgpu;

__global__ void TestHwRegIDs(HwRegIDs *id)
{
  auto gid = blockDim.x * blockIdx.x + threadIdx.x;
  if (gid) return;

  *id = HwRegIDs();
}

int main()
{
  auto default_hwid = HwRegIDs();

  if (default_hwid.GfxArch().Family() != ArchFamily::INVALID) {
    throw StatusError(Status::Code::ERROR);
  }

  auto constructed = HwRegIDs(ArchFamily::GFX940Plus, {0xdeadbeef, 0xbadc0de});
  if (constructed.HwID() != 0xdeadbeef) {
    throw StatusError(Status::Code::ERROR);
  }

  if (constructed.XccID() != 0xe) {
    throw StatusError(Status::Code::ERROR);
  }

  auto hwid = host::make_unique<HwRegIDs>();

  TestHwRegIDs<<<1, 64>>>(hwid.get());
  HIP_CHECK(hipGetLastError());
  HIP_CHECK(hipDeviceSynchronize());

  std::cout << std::string(*hwid) << "\n";

  //  This is a static hwid test based on previously logged hwid reg reads
  //  family:GFX10Plus hwid0:0x00040001 hwid1:0x08000102 wave:1 simd:0 wgp:0
  //  sa:0 se:1 queue:2 pipe:0 me:1 state:0 wg:0 vm:8 compat_level:0
  auto navi10 = HwRegIDs(ArchFamily::GFX10Plus, {0x40001, 0x8000102});
  if (navi10.SaID() != 0) {
    throw StatusError(Status::Code::ERROR);
  }
  if (navi10.SeID() != 1) {
    throw StatusError(Status::Code::ERROR);
  }
  if (navi10.QueueID() != 2) {
    throw StatusError(Status::Code::ERROR);
  }

  if (navi10.PipeID() != 0) {
    throw StatusError(Status::Code::ERROR);
  }

  if (navi10.StateID() != 0) {
    throw StatusError(Status::Code::ERROR);
  }

  if (navi10.WgID() != 0) {
    throw StatusError(Status::Code::ERROR);
  }

  if (navi10.WgpID() != 0) {
    throw StatusError(Status::Code::ERROR);
  }

  if (navi10.VmID() != 8) {
    throw StatusError(Status::Code::ERROR);
  }

  if (navi10.CompatLevelID() != 0) {
    throw StatusError(Status::Code::ERROR);
  }

  return 0;
}
