/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <hip/hip_runtime.h>
#include "embers/amdgpu/chipid.h"
#include "embers/memory.h"
#include "test_helpers.h"
#include <iomanip>
#include <pciaccess.h>

using namespace embers;
using namespace embers::amdgpu;

__global__ void CheckChipID(ChipID id, bool *untested, bool *valid)
{
  if ((blockIdx.x == 0) && (threadIdx.x == 0)) {
#if defined(__gfx906__)
    if (!IsVEGA20(id)) *valid = false;
    return;
#endif
#if defined(__gfx908__)
    if (!IsMI100(id)) *valid = false;
    return;
#endif

#if defined(__gfx90a__)
    if (!IsMI200(id)) *valid = false;
    return;
#endif

#if defined(__gfx940__) || defined(__gfx941__) || defined(__gfx942__)
    if (!IsMI3XX(id)) *valid = false;
    return;
#endif

#if defined(__gfx940__)
    if (!IsMI300A(id)) *valid = false;
    return;
#endif

#if defined(__gfx941__)
    if (!IsMI300X(id)) *valid = false;
    return;
#endif

#if defined(__gfx942__)
    if (!IsMI300X(id) || !IsMI308X(id) || !IsMI325X(id) || !IsMI300XHF(id)) *valid = false;
    return;
#endif

#if defined(__gfx1010__)
    if (!IsNAVI10(id)) *valid = false;
    return;
#endif

#if defined(__gfx1030__)
    if (!IsNAVI21(id)) *valid = false;
    return;
#endif

    *untested = true;
  }
}

int main()
{
  bool error_detected = false;
  int dev_count;
  HIP_CHECK(hipGetDeviceCount(&dev_count));

  auto err = pci_system_init();
  if (err) {
    std::cerr << "failed to init pci system\n";
    return -1;
  }

  for (auto dev = 0; dev < dev_count; dev++) {
    auto untested = host::make_unique<bool>(false);
    auto valid = host::make_unique<bool>(true);

    hipDeviceProp_t props;

    HIP_CHECK(hipSetDevice(dev));

    HIP_CHECK(hipGetDeviceProperties(&props, dev));
    struct pci_device *pcidev = nullptr;
    pcidev = pci_device_find_by_slot(props.pciDomainID, props.pciBusID, props.pciDeviceID,
                                     0 /*func*/);
    if (!pcidev) {
      std::cout << "WARN: Failed to find pci device matching dev " << dev << std::endl;
      continue;
    }
    int chip_id = pcidev->device_id;

    std::cout << "Checking that Dev: " << std::dec << " has ChipID 0x" << std::hex << chip_id
              << "\n";
    CheckChipID<<<1, 1>>>(ChipID(chip_id), untested.get(), valid.get());
    HIP_CHECK(hipGetLastError());
    HIP_CHECK(hipDeviceSynchronize());
    if (*untested) {
      std::cout << "WARN: Device with ChipID 0x" << std::hex << std::setw(8) << std::setfill('0')
                << chip_id << " untested"
                << "\n";
      continue;
    }
    if (!*valid) {
      std::cerr << "ERROR: Device with ChipID 0x" << std::hex << std::setw(8) << std::setfill('0')
                << chip_id << " failed check"
                << "\n";
      error_detected = true;
    }
  }
  return error_detected ? 1 : 0;
}
