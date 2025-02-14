/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_BACKOFF_H_
#define _EMBERS_BACKOFF_H_

#include <thread>
#include <hip/hip_runtime.h>

namespace embers
{

__host__ inline void backoff() noexcept { std::this_thread::yield(); }
__device__ inline void backoff() noexcept { __builtin_amdgcn_s_sleep(127); }

}  // namespace embers
#endif  // _EMBERS_BACKOFF_H_
