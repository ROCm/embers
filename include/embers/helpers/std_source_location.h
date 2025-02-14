/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_SOURCE_LOCATION_H_
#define _EMBERS_SOURCE_LOCATION_H_

#include <cstdint>

#define _EMBERS_HAVE_STD_SOURCE_LOCATION __has_include(<source_location>)
#define _EMBERS_HAVE_STD_EXP_SOURCE_LOCATION __has_include(<experimental/source_location>)

namespace embers
{

/// @cond
// Keep this outside of the #ifdefery so that the code is always compiled and
// checked for errors.
struct __std_source_location {
  static constexpr __std_source_location current() noexcept { return __std_source_location{}; }

  constexpr __std_source_location() noexcept {};
  constexpr uint_least32_t line() const noexcept { return 0; };
  constexpr const char *file_name() const noexcept { return "none"; };
  constexpr const char *function_name() const noexcept { return "none"; };
};
/// @endcond

// __has_include() can only be used in preprocessor context but we still want a
// runtime constant to indicate support for std::source_location.
#if (_EMBERS_HAVE_STD_SOURCE_LOCATION || _EMBERS_HAVE_STD_EXP_SOURCE_LOCATION)

inline constexpr bool HAVE_STD_SOURCE_LOCATION = true;

#else

inline constexpr bool HAVE_STD_SOURCE_LOCATION = false;

#endif

}  // namespace embers

#if _EMBERS_HAVE_STD_SOURCE_LOCATION

#include <source_location>

namespace embers
{
using std_source_location = std::source_location;
}

#elif _EMBERS_HAVE_STD_EXP_SOURCE_LOCATION

#include <experimental/source_location>
namespace embers
{
using std_source_location = std::experimental::source_location;
}

#else

namespace embers
{
using std_source_location = __std_source_location;
}

#endif

#endif  //_EMBERS_SOURCE_LOCATION_H_
