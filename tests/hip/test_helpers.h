/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_HIP_TEST_HELPERS_H_
#define _EMBERS_HIP_TEST_HELPERS_H_

#include <iostream>

// HIP error check
#define HIP_CHECK(command)                                                                    \
  {                                                                                           \
    hipError_t stat = (command);                                                              \
    if (stat != hipSuccess) {                                                                 \
      std::cerr << "HIP error: " << hipGetErrorString(stat) << " in file " << __FILE__ << ":" \
                << __LINE__ << std::endl;                                                     \
      exit(-1);                                                                               \
    }                                                                                         \
  }

#endif  // _EMBERS_HIP_TEST_HELPERS_H_
