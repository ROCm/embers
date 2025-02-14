/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_ALMOST_EQUAL_IMPL_
#define _EMBERS_ALMOST_EQUAL_IMPL_

#include <cmath>
#include <limits>
#include <type_traits>

namespace embers
{

template <typename T>
bool AlmostEqualRelative(T a, T b)
{
  if constexpr (std::is_floating_point<T>::value) {
    const T diff = std::abs(a - b);
    a = std::abs(a);
    b = std::abs(b);
    const T larger = (b > a) ? b : a;
    return diff <= larger * std::numeric_limits<T>::epsilon();
  } else {
    return a == b;
  }
}

template <typename T>
bool AlmostEqualAbsolute(T a, T b)
{
  if constexpr (std::is_floating_point<T>::value) {
    return std::abs(a - b) <= std::numeric_limits<T>::epsilon();
  } else {
    return a == b;
  }
}

}  // namespace embers

#endif
