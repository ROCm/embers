/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <cstdlib>
#include <limits>
#include <iostream>

#include "embers/helpers/almost_equal.h"

using namespace embers;

#define test_assert(expr)                                                     \
  if (!(expr)) {                                                              \
    std::cerr << "Test failed: " << #expr << " at line " << __LINE__ << "\n"; \
    std::abort();                                                             \
  }

class Composition
{
 private:
  int a;
  int b;
  int c;

 public:
  constexpr Composition(int a, int b, int c) : a(a), b(b), c(c) {}
  constexpr int count() const { return a + b + c; }
  constexpr bool operator==(const Composition &other) const { return count() == other.count(); }
};

template <typename T>
constexpr void GenericTestCases()
{
  test_assert(AlmostEqualAbsolute<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::min()));
  test_assert(AlmostEqualRelative<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max()));

  test_assert(AlmostEqualAbsolute<T>(0, 0));
  test_assert(AlmostEqualRelative<T>(0, 0));
}

template <typename T>
constexpr void FloatingPointTestCases()
{
  static_assert(std::is_floating_point<T>::value);
  constexpr T lowest = std::numeric_limits<T>::lowest();
  constexpr T max = std::numeric_limits<T>::max();
  constexpr T min = std::numeric_limits<T>::min();
  constexpr T epsilon = std::numeric_limits<T>::epsilon();

  // Very large/small numbers
  // Changing by one exponent value should be rejected.
  test_assert(!AlmostEqualRelative<T>(lowest, lowest * static_cast<T>(0.1)));
  test_assert(!AlmostEqualRelative<T>(max, max * static_cast<T>(0.1)));
  // However, chaning by 1 in the mantissa is indistinguishable for large
  // numbers.
  test_assert(AlmostEqualRelative<T>(lowest, lowest + static_cast<T>(1)));
  test_assert(AlmostEqualRelative<T>(max, max - static_cast<T>(1)));

  // Very close to 0. Epsilon is the decision threshold.
  test_assert(!AlmostEqualAbsolute<T>(-epsilon, epsilon));
  test_assert(AlmostEqualAbsolute<T>(-min, min));
}

int main()
{
  GenericTestCases<long double>();
  GenericTestCases<double>();
  GenericTestCases<float>();
  GenericTestCases<int>();
  GenericTestCases<unsigned int>();

  FloatingPointTestCases<long double>();
  FloatingPointTestCases<double>();
  FloatingPointTestCases<float>();

  // Check that objects with custom comparison operators can be compared
  test_assert(AlmostEqualAbsolute<Composition>({0, 1, 2}, {1, 1, 1}));
  test_assert(AlmostEqualRelative<Composition>({0, 1, 2}, {1, 1, 1}));
  test_assert(!AlmostEqualAbsolute<Composition>({0, 1, 2}, {1, 1, 3}));
  test_assert(!AlmostEqualRelative<Composition>({0, 1, 2}, {1, 1, 3}));
}
