/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_ALMOST_EQUAL_
#define _EMBERS_ALMOST_EQUAL_

namespace embers
{

// @brief Compares two numbers for near-equality
// @param a First number to compare
// @param b Second number to compare
// \return True if the numbers are relatively equal
// \note Use AlmostEqualRelative for comparisons far from 0
template <typename T>
inline bool AlmostEqualRelative(T a, T b);

// @brief Compares two numbers for near-equality
// @param a First number to compare
// @param b Second number to compare
// \return True if the numbers are relatively equal
// \note Use AlmostEqualAbsolute for comparisons near 0
template <typename T>
inline bool AlmostEqualAbsolute(T a, T b);

}  // namespace embers

#include "almost_equal_impl.h"

#endif
