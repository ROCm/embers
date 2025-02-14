/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef HWID_H
#define HWID_H

#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

#include <hip/hip_runtime.h>

#include "embers/status.h"
#include "embers/amdgpu/arch.h"
#include "embers/helpers/bit_helpers.cuh"

namespace embers
{
namespace amdgpu
{

class HwRegIDs
{
 public:
  static constexpr int MAX_NUM_HWID_REGS = 2;

 private:
  static constexpr unsigned short GFX9_ME_MSB = 31;
  static constexpr unsigned short GFX9_STATE_MSB = 29;
  static constexpr unsigned short GFX9_ME_LSB = 30;
  static constexpr unsigned short GFX9_STATE_LSB = 27;
  static constexpr unsigned short GFX9_QUEUE_MSB = 26;
  static constexpr unsigned short GFX9_QUEUE_LSB = 24;
  static constexpr unsigned short GFX9_VM_MSB = 23;
  static constexpr unsigned short GFX9_VM_LSB = 20;
  static constexpr unsigned short GFX9_TG_MSB = 19;
  static constexpr unsigned short GFX9_TG_LSB = 16;
  static constexpr unsigned short GFX9_SE_MSB = 14;
  static constexpr unsigned short GFX9_SE_LSB = 13;
  static constexpr unsigned short GFX9_SH_MSB = 12;
  static constexpr unsigned short GFX9_SH_LSB = 12;
  static constexpr unsigned short GFX9_CU_MSB = 11;
  static constexpr unsigned short GFX9_CU_LSB = 8;
  static constexpr unsigned short GFX9_PIPE_MSB = 7;
  static constexpr unsigned short GFX9_PIPE_LSB = 6;
  static constexpr unsigned short GFX9_SIMD_MSB = 5;
  static constexpr unsigned short GFX9_SIMD_LSB = 4;
  static constexpr unsigned short GFX9_WAVE_MSB = 3;
  static constexpr unsigned short GFX9_WAVE_LSB = 0;

  // GFX940Plus
  static constexpr unsigned short GFX940P_XCC_MSB = 3;
  static constexpr unsigned short GFX940P_XCC_LSB = 0;

  // GFX10Plus SQ_WAVE_HW_ID1
  static constexpr unsigned short GFX10P_SE_MSB = 19;
  static constexpr unsigned short GFX10P_SE_LSB = 18;
  static constexpr unsigned short GFX10P_SA_MSB = 16;
  static constexpr unsigned short GFX10P_SA_LSB = 16;
  static constexpr unsigned short GFX10P_WGP_MSB = 13;
  static constexpr unsigned short GFX10P_WGP_LSB = 10;
  static constexpr unsigned short GFX10P_SIMD_MSB = 9;
  static constexpr unsigned short GFX10P_SIMD_LSB = 8;
  static constexpr unsigned short GFX10P_WAVE_MSB = 4;
  static constexpr unsigned short GFX10P_WAVE_LSB = 0;

  // GFX10Plus SQ_WAVE_HW_ID2
  static constexpr unsigned short GFX10P_COMPAT_LEVEL_MSB = 30;
  static constexpr unsigned short GFX10P_COMPAT_LEVEL_LSB = 29;
  static constexpr unsigned short GFX10P_VM_MSB = 27;
  static constexpr unsigned short GFX10P_VM_LSB = 24;
  static constexpr unsigned short GFX10P_WG_MSB = 20;
  static constexpr unsigned short GFX10P_WG_LSB = 16;
  static constexpr unsigned short GFX10P_STATE_MSB = 14;
  static constexpr unsigned short GFX10P_STATE_LSB = 12;
  static constexpr unsigned short GFX10P_ME_MSB = 9;
  static constexpr unsigned short GFX10P_ME_LSB = 8;
  static constexpr unsigned short GFX10P_PIPE_MSB = 5;
  static constexpr unsigned short GFX10P_PIPE_LSB = 4;
  static constexpr unsigned short GFX10P_QUEUE_MSB = 3;
  static constexpr unsigned short GFX10P_QUEUE_LSB = 0;

  // GFX11Plus HW_ID1
  static constexpr unsigned short GFX11P_DP_RATE_MSB = 31;
  static constexpr unsigned short GFX11P_DP_RATE_LSB = 29;
  static constexpr unsigned short GFX11P_SE_MSB = 20;
  static constexpr unsigned short GFX11P_SE_LSB = 18;
  static constexpr unsigned short GFX11P_SA_MSB = 16;
  static constexpr unsigned short GFX11P_SA_LSB = 16;
  static constexpr unsigned short GFX11P_WGP_MSB = 13;
  static constexpr unsigned short GFX11P_WGP_LSB = 10;
  static constexpr unsigned short GFX11P_SIMD_MSB = 9;
  static constexpr unsigned short GFX11P_SIMD_LSB = 8;
  static constexpr unsigned short GFX11P_WAVE_MSB = 4;
  static constexpr unsigned short GFX11P_WAVE_LSB = 0;

  // GFX11Plus HW_ID2
  static constexpr unsigned short GFX11P_VM_MSB = 27;
  static constexpr unsigned short GFX11P_VM_LSB = 24;
  static constexpr unsigned short GFX11P_WG_MSB = 20;
  static constexpr unsigned short GFX11P_WG_LSB = 16;
  static constexpr unsigned short GFX11P_STATE_MSB = 14;
  static constexpr unsigned short GFX11P_STATE_LSB = 12;
  static constexpr unsigned short GFX11P_ME_MSB = 9;
  static constexpr unsigned short GFX11P_ME_LSB = 8;
  static constexpr unsigned short GFX11P_PIPE_MSB = 5;
  static constexpr unsigned short GFX11P_PIPE_LSB = 4;
  static constexpr unsigned short GFX11P_QUEUE_MSB = 3;
  static constexpr unsigned short GFX11P_QUEUE_LSB = 0;

  std::array<uint32_t, MAX_NUM_HWID_REGS> raw_;
  GFXArch gfx_arch_;
  // raw[1] used for xcc ID in MI300 and for HWID reg 2 for GFX10PLus
  // GFX9

  /// @brief sets the Architecture
  __device__ inline void SetArch() noexcept
  {
#if defined(__gfx900__) || defined(__gfx906__) || defined(__gfx908__) || defined(__gfx90a__)
    gfx_arch_ = GFXArch(ArchFamily::GFX9);
#elif defined(__gfx1010__) || defined(__gfx1011__) || defined(__gfx1012__) || \
    defined(__gfx1030__) || defined(__gfx1031__) || defined(__gfx1032__) ||   \
    defined(__gfx1033__) || defined(__gfx1034__) || defined(__gfx1035__) || defined(__gfx1036__)
    gfx_arch_ = GFXArch(ArchFamily::GFX10Plus);
#elif defined(__gfx1100__) || defined(__gfx1101__) || defined(__gfx1102__) || defined(__gfx1103__)
    gfx_arch_ = GFXArch(ArchFamily::GFX11Plus);
#elif defined(__gfx940__) || defined(__gfx941__) || defined(__gfx942__)
    gfx_arch_ = GFXArch(ArchFamily::GFX940Plus);
#elif defined(__HIP_DEVICE_COMPILE__)
    static_assert(false, "This GFX_ARCH is not supported");
#endif
  }

  /// @brief sets the Architecture
  __host__ inline void SetArch() noexcept { gfx_arch_ = GFXArch(ArchFamily::INVALID); }

  /// @brief gets the latest HWID reg values from hardware
  __device__ inline void ReadHwIDRegs() noexcept
  {
#if defined(__gfx900__) || defined(__gfx906__) || defined(__gfx908__) || defined(__gfx90a__)

    asm volatile("s_getreg_b32 %0, hwreg(HW_REG_HW_ID, 0, 32)" : "=r"(raw_[0])::);
    raw_[1] = 0;

#elif defined(__gfx1010__) || defined(__gfx1011__) || defined(__gfx1012__) || \
    defined(__gfx1030__) || defined(__gfx1031__) || defined(__gfx1032__) ||   \
    defined(__gfx1033__) || defined(__gfx1034__) || defined(__gfx1035__) || defined(__gfx1036__)
    asm volatile("s_getreg_b32 %0, hwreg(HW_REG_HW_ID, 0, 32)" : "=r"(raw_[0])::);
    asm volatile("s_getreg_b32 %0, hwreg(HW_REG_HW_ID2, 0, 32)" : "=r"(raw_[1])::);

#elif defined(__gfx1100__) || defined(__gfx1101__) || defined(__gfx1102__) || defined(__gfx1103__)

    asm volatile("s_getreg_b32 %0, hwreg(HW_REG_HW_ID1, 0, 32)" : "=r"(raw_[0])::);
    asm volatile("s_getreg_b32 %0, hwreg(HW_REG_HW_ID2, 0, 32)" : "=r"(raw_[1])::);

#elif defined(__gfx940__) || defined(__gfx941__) || defined(__gfx942__)

    gfx_arch_ = GFXArch(ArchFamily::GFX940Plus);
    asm volatile("s_getreg_b32 %0, hwreg(HW_REG_HW_ID, 0, 32)" : "=r"(raw_[0])::);
    asm volatile("s_getreg_b32 %0, hwreg(HW_REG_XCC_ID, 0, 32)" : "=r"(raw_[1])::);

#elif defined(__HIP_DEVICE_COMPILE__)
    static_assert(false, "This GFX_ARCH is not supported");
#endif
  }

  /// @brief stub for host side
  __host__ inline void ReadHwIDRegs() volatile noexcept {}

  /// @brief helper for host constructor
  __host__ void InitState()
  {
    SetArch();
    raw_.fill(0);
  }

  /// @brief helper for device constructor
  __device__ void InitState()
  {
    SetArch();
    ReadHwIDRegs();
  }

  __host__ void FeatureNotSupported()
  {
    throw StatusError(Status::Code::ERROR, "HwRegIDs feature not supported for this arch");
  }
  __device__ void FeatureNotSupported() { abort(); }

 public:
  /// @brief HwID default constructor for host and device code
  __host__ __device__ HwRegIDs() { InitState(); }

  /// @brief HwID constructor for host code code
  ///
  /// @param array of HW_REG_ID values
  __host__ HwRegIDs(ArchFamily family, std::array<uint32_t, MAX_NUM_HWID_REGS> regvals)
      : raw_(regvals), gfx_arch_(GFXArch(family))
  {
  }

  /// @brief returns the current HW_REG_HW_ID register value
  __host__ __device__ uint32_t HwID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return raw_.at(0);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief returns the current HW_REG_HW_ID1 register value
  __host__ __device__ uint32_t HwID1()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX10Plus:
        return raw_.at(0);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief returns the current HW_REG_HW_ID2 register value
  __host__ __device__ uint32_t HwID2()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX10Plus:
        return raw_.at(1);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the GFXArch
  __host__ __device__ GFXArch GfxArch() const noexcept { return gfx_arch_; }

  /// @brief Returns the wavefront ID
  __host__ __device__ uint8_t WaveID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_WAVE_MSB, GFX9_WAVE_LSB);
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[0], GFX10P_WAVE_MSB, GFX10P_WAVE_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[0], GFX11P_WAVE_MSB, GFX11P_WAVE_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the wavefront SIMD ID
  __host__ __device__ uint8_t SimdID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_SIMD_MSB, GFX9_SIMD_LSB);
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[0], GFX10P_SIMD_MSB, GFX10P_SIMD_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[0], GFX11P_SIMD_MSB, GFX11P_SIMD_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the wavefront Pipe ID
  __host__ __device__ uint8_t PipeID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_PIPE_MSB, GFX9_PIPE_LSB);
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[1], GFX10P_PIPE_MSB, GFX10P_PIPE_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[1], GFX11P_PIPE_MSB, GFX11P_PIPE_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the CU ID
  __host__ __device__ uint8_t CuID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_CU_MSB, GFX9_CU_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the SH ID
  __host__ __device__ uint8_t ShID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_SH_MSB, GFX9_SH_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the SE ID
  __host__ __device__ uint8_t SeID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_SE_MSB, GFX9_SE_LSB);
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[0], GFX10P_SE_MSB, GFX10P_SE_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[0], GFX11P_SE_MSB, GFX11P_SE_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the DP rate
  __host__ __device__ uint8_t DPRate()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[0], GFX11P_DP_RATE_MSB, GFX11P_DP_RATE_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the TG ID
  __host__ __device__ uint8_t TgID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_TG_MSB, GFX9_TG_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the VMID
  __host__ __device__ uint8_t VmID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_VM_MSB, GFX9_VM_LSB);
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[1], GFX10P_VM_MSB, GFX10P_VM_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[1], GFX11P_VM_MSB, GFX11P_VM_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the Queue ID
  __host__ __device__ uint8_t QueueID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_QUEUE_MSB, GFX9_QUEUE_LSB);
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[1], GFX10P_QUEUE_MSB, GFX10P_QUEUE_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[1], GFX11P_QUEUE_MSB, GFX11P_QUEUE_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the State ID
  __host__ __device__ uint8_t StateID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_STATE_MSB, GFX9_STATE_LSB);
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[1], GFX10P_STATE_MSB, GFX10P_STATE_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[1], GFX11P_STATE_MSB, GFX11P_STATE_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the ME ID
  __host__ __device__ uint8_t MeID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX9:
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[0], GFX9_ME_MSB, GFX9_ME_LSB);
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[1], GFX10P_ME_MSB, GFX10P_ME_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[1], GFX11P_ME_MSB, GFX11P_ME_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the WG ID
  __host__ __device__ uint8_t WgID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[1], GFX10P_WG_MSB, GFX10P_WG_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[1], GFX11P_WG_MSB, GFX11P_WG_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the CompatLevel ID
  __host__ __device__ uint8_t CompatLevelID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[1], GFX10P_COMPAT_LEVEL_MSB, GFX10P_COMPAT_LEVEL_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the WGP ID
  __host__ __device__ uint8_t WgpID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[0], GFX10P_WGP_MSB, GFX10P_WGP_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[0], GFX11P_WGP_MSB, GFX11P_WGP_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the SA ID
  __host__ __device__ uint8_t SaID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX10Plus:
        return get_bits(raw_[0], GFX10P_SA_MSB, GFX10P_SA_LSB);
      case ArchFamily::GFX11Plus:
        return get_bits(raw_[0], GFX11P_SA_MSB, GFX11P_SA_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  /// @brief Returns the XCC ID
  __host__ __device__ uint8_t XccID()
  {
    ReadHwIDRegs();
    switch (gfx_arch_.Family()) {
      case ArchFamily::GFX940Plus:
        return get_bits(raw_[1], GFX940P_XCC_MSB, GFX940P_XCC_LSB);
      default:
        FeatureNotSupported();
        return 0;
    }
  }

  operator std::string()
  {
    ReadHwIDRegs();
    std::stringstream ss;
    switch (gfx_arch_.Family()) {
      case (ArchFamily::GFX9):
        ss << "family:" << std::string(gfx_arch_) << " hwid:0x" << std::setfill('0') << std::setw(8)
           << std::hex << raw_[0];
        ss << std::dec << " wave:" << std::to_string(WaveID())
           << " simd:" << std::to_string(SimdID()) << " pipe:";
        ss << std::to_string(PipeID()) << " cu:" << std::to_string(CuID())
           << " sh:" << std::to_string(ShID());
        ss << " se:" << std::to_string(SeID()) << " tg:" << std::to_string(TgID())
           << " vm:" << std::to_string(VmID());
        ss << " queue:" << std::to_string(QueueID()) << " state:" << std::to_string(StateID())
           << " me:0x" << std::hex << static_cast<unsigned int>(MeID());
        return ss.str();
      case (ArchFamily::GFX940Plus):
        ss << "family:" << std::string(gfx_arch_) << " hwid:0x" << std::setfill('0') << std::setw(8)
           << std::hex << raw_[0];
        ss << std::dec << " wave:" << std::to_string(WaveID())
           << " simd:" << std::to_string(SimdID()) << " pipe:";
        ss << std::to_string(PipeID()) << " cu:" << std::to_string(CuID())
           << " sh:" << std::to_string(ShID());
        ss << " se:" << std::to_string(SeID()) << " tg:" << std::to_string(TgID())
           << " vm:" << std::to_string(VmID());
        ss << " queue:" << std::to_string(QueueID()) << " state:" << std::to_string(StateID())
           << " me:0x" << std::hex << static_cast<unsigned int>(MeID());
        ss << std::dec << " xcc:" << std::to_string(XccID());
        return ss.str();
      case (ArchFamily::GFX10Plus):
        ss << "family:" << std::string(gfx_arch_) << " hwid:0x" << std::setfill('0') << std::setw(8)
           << std::hex << raw_[0];
        ss << " hwid1:0x" << std::setfill('0') << std::setw(8) << std::hex << raw_[1];
        ss << std::dec << " wave:" << std::to_string(WaveID())
           << " simd:" << std::to_string(SimdID()) << " wgp:" << std::to_string(WgpID());
        ss << " sa:" << std::to_string(SaID()) << " se:" << std::to_string(SeID())
           << " queue:" << std::to_string(QueueID());
        ss << " pipe:" << std::to_string(PipeID()) << " me:" << std::to_string(MeID())
           << " state:" << std::to_string(StateID());
        ss << " wg:" << std::to_string(WgID()) << " vm:" << std::to_string(VmID())
           << " compat_level:" << std::to_string(CompatLevelID());
        return ss.str();
      case (ArchFamily::GFX11Plus):
        ss << "family:" << std::string(gfx_arch_) << " hwid:0x" << std::setfill('0') << std::setw(8)
           << std::hex << raw_[0];
        ss << " hwid1:0x" << std::setfill('0') << std::setw(8) << std::hex << raw_[1];
        ss << std::dec << " wave:" << std::to_string(WaveID())
           << " simd:" << std::to_string(SimdID()) << " wgp:" << std::to_string(WgpID());
        ss << " sa:" << std::to_string(SaID()) << " se:" << std::to_string(SeID())
           << " queue:" << std::to_string(QueueID());
        ss << " pipe:" << std::to_string(PipeID()) << " me:" << std::to_string(MeID())
           << " state:" << std::to_string(StateID());
        ss << " wg:" << std::to_string(WgID()) << " vm:" << std::to_string(VmID())
           << " dprate:" << std::to_string(DPRate());
        return ss.str();
      default:
        return "gfx HwID parsing not available for this gfx arch";
    }
  }
};

}  // namespace amdgpu
}  // namespace embers

#endif
