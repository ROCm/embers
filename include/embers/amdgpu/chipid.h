/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_AMDGPU_CHIPID_H_
#define _EMBERS_AMDGPU_CHIPID_H_

#include <hip/hip_runtime.h>
namespace embers
{
namespace amdgpu
{

/// @brief Enum representing AMD GPU PCI device IDs
enum class ChipID : int {
  NAVI10_W5700X = 0x7310,
  NAVI10_W5700 = 0x7312,
  NAVI10_5700 = 0x731b,
  NAVI10_5600 = 0x731f,
  NAVI21_V620 = 0x73a1,
  NAVI21_W6900X = 0x73a2,
  NAVI21_W6800 = 0x73a3,
  NAVI21_6950XT = 0x73a5,
  NAVI21_W6800X = 0x73ab,
  NAVI21_V620MX = 0x73ae,
  NAVI21_6900XT = 0x73af,
  NAVI21_6800XT = 0x73bf,
  NAVI31_W7900 = 0x7448,
  NAVI31_7900XT = 0x744c,
  NAVI31_W7800 = 0x745e,
  VEGA20_INSTINCT = 0x66a0,
  VEGA20_MI50 = 0x66a1,
  VEGA20 = 0x66a2,
  VEGA20_VEGAII = 0x66a3,
  VEGA20_VII = 0x66af,
  MI100_0 = 0x7388,
  MI100_1 = 0x738c,
  MI100_2 = 0x738e,
  MI210 = 0x740F,
  MI250X = 0x7408,
  MI250X_MI250 = 0x740c,
  MI300X = 0x74a1,
  MI300X_SRIOV = 0x74b5,
  MI300X_HF = 0x74a9,
  MI300X_HF_SRIOV = 0x74bd,
  MI300A = 0x74a0,
  MI300A_SRIOV = 0x74b4,
  MI308X = 0x74a2,
  MI308X_SRIOV = 0x74b6,
  MI325X = 0x74a5,
  MI325X_SRIOV = 0x74b9,
};

/// @brief Returns true if the id passed in matches a VEGA20 PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsVEGA20(ChipID id)
{
  switch (id) {
    case ChipID::VEGA20_INSTINCT:
      return true;
    case ChipID::VEGA20_MI50:
      return true;
    case ChipID::VEGA20:
      return true;
    case ChipID::VEGA20_VEGAII:
      return true;
    case ChipID::VEGA20_VII:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a MI100 PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI100(ChipID id)
{
  switch (id) {
    case ChipID::MI100_0:
      return true;
    case ChipID::MI100_1:
      return true;
    case ChipID::MI100_2:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a NAVI10 PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsNAVI10(ChipID id)
{
  switch (id) {
    case ChipID::NAVI10_W5700X:
      return true;
    case ChipID::NAVI10_W5700:
      return true;
    case ChipID::NAVI10_5700:
      return true;
    case ChipID::NAVI10_5600:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a NAVI21 PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsNAVI21(ChipID id)
{
  switch (id) {
    case ChipID::NAVI21_V620:
      return true;
    case ChipID::NAVI21_W6900X:
      return true;
    case ChipID::NAVI21_W6800:
      return true;
    case ChipID::NAVI21_6950XT:
      return true;
    case ChipID::NAVI21_W6800X:
      return true;
    case ChipID::NAVI21_V620MX:
      return true;
    case ChipID::NAVI21_6900XT:
      return true;
    case ChipID::NAVI21_6800XT:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a NAVI31 PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsNAVI31(ChipID id)
{
  switch (id) {
    case ChipID::NAVI21_V620:
      return true;
    case ChipID::NAVI21_W6900X:
      return true;
    case ChipID::NAVI21_W6800:
      return true;
    case ChipID::NAVI21_6950XT:
      return true;
    case ChipID::NAVI21_W6800X:
      return true;
    case ChipID::NAVI21_V620MX:
      return true;
    case ChipID::NAVI21_6900XT:
      return true;
    case ChipID::NAVI21_6800XT:
      return true;
    case ChipID::NAVI31_W7900:
      return true;
    case ChipID::NAVI31_7900XT:
      return true;
    case ChipID::NAVI31_W7800:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a MI200 PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI200(ChipID id)
{
  switch (id) {
    case ChipID::MI210:
      return true;
    case ChipID::MI250X:
      return true;
    case ChipID::MI250X_MI250:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a MI300X PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI300X(ChipID id)
{
  switch (id) {
    case ChipID::MI300X:
      return true;
    case ChipID::MI300X_SRIOV:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a MI300XHF PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI300XHF(ChipID id)
{
  switch (id) {
    case ChipID::MI300X_HF:
      return true;
    case ChipID::MI300X_HF_SRIOV:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a MI300A PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI300A(ChipID id)
{
  switch (id) {
    case ChipID::MI300A:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a MI308X PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI308X(ChipID id)
{
  switch (id) {
    case ChipID::MI308X:
      return true;
    case ChipID::MI308X_SRIOV:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a MI325X PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI325X(ChipID id)
{
  switch (id) {
    case ChipID::MI325X:
      return true;
    case ChipID::MI325X_SRIOV:
      return true;
    default:
      return false;
  }
}

/// @brief Returns true if the id passed in matches a MI300 PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI300(ChipID id)
{
  return IsMI300A(id) || IsMI300X(id) || IsMI300XHF(id);
}

/// @brief Returns true if the id passed in matches a MI3XX PCI Device ID
///
/// @param id ChipID to evaluate
/// @returns a bool (true if ID matches, false otherwise)
__host__ __device__ inline bool IsMI3XX(ChipID id)
{
  return IsMI300(id) || IsMI308X(id) || IsMI325X(id) || IsMI300XHF(id);
}

}  // namespace amdgpu
}  // namespace embers

#endif  // _EMBERS_AMDGPU_CHIPID_H_
