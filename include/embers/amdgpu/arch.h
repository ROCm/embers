/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_AMDGPU_ARCH_H_
#define _EMBERS_AMDGPU_ARCH_H_

#include <string>

#include <hip/hip_runtime.h>

namespace embers
{
namespace amdgpu
{

/// @brief Represents the Architecture Family of an AMDGPU Device
enum class ArchFamily {
  INVALID = 0,
  GFX9 = 1,
  GFX940Plus = 2,
  GFX10Plus = 3,
  GFX11Plus = 4,
};

/// @brief Represents the Architecture an AMDGPU Device
class GFXArch
{
 private:
  ArchFamily family_;

 public:
  /// @brief Default constructor
  __host__ __device__ GFXArch() : family_(ArchFamily::INVALID) {}

  /// @brief Construct a GFXARch from an ArchFamily enum
  __host__ __device__ GFXArch(ArchFamily family) : family_(family) {}

  /// @brief return the GFXArch as a string. For now simply returns the family
  /// arch name.
  __host__ __device__ operator std::string() const noexcept
  {
    switch (family_) {
      case ArchFamily::GFX9:
        return "GFX9";
      case ArchFamily::GFX940Plus:
        return "GFX940Plus";
      case ArchFamily::GFX10Plus:
        return "GFX10Plus";
      case ArchFamily::GFX11Plus:
        return "GFX11Plus";
      default:
        return "INVALID";
    }
  }

  /// @brief return the ArchFamily of this GFXArch
  __host__ __device__ ArchFamily Family() const noexcept { return family_; }
};

}  // namespace amdgpu
}  // namespace embers
#endif  //  _EMBERS_AMDGPU_ARCH_H_
