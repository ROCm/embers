/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_FIRESTORM_DETAIL_MEMORY_H_
#define _EMBERS_FIRESTORM_DETAIL_MEMORY_H_

#include <new>
#include <hip/hip_runtime.h>

#include "embers/memory/unique_ptr.h"
#include "embers/status.h"

namespace embers
{

namespace device
{

///! @cond
template <class T, class... Args>
inline typename _Unique_if<T>::_Single_object make_unique_with_attributes(int hip_device,
                                                                          unsigned int flags,
                                                                          Args &&...args)
{
  return typename _Unique_if<T>::_Single_object(
      [&]() {
        T *buf;
        int old;
        auto err = hipGetDevice(&old);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }

        err = hipSetDevice(hip_device);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
        err = hipExtMallocWithFlags(reinterpret_cast<void **>(&buf), sizeof(T), flags);
        if (err != hipSuccess) {
          auto fail_msg = hipGetErrorString(err);
          err = hipSetDevice(old);
          if (err != hipSuccess) {
            auto setdev_fail_msg = hipGetErrorString(err);
            throw StatusError(Status::Code::NO_FREE_RESOURCE,
                              (std::string(fail_msg) + std::string(", ") +
                               std::string(setdev_fail_msg))
                                  .c_str());
          }
          throw StatusError(Status::Code::NO_FREE_RESOURCE, hipGetErrorString(err));
        }
        err = hipSetDevice(old);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
        if constexpr (std::is_constructible<T, Args...>::value) {
          new (buf) T(std::forward<Args &&>(args)...);
        } else {
          new (buf) T{std::forward<Args &&>(args)...};
        }
        return buf;
      }(),
      [](T *buf) {
        buf->~T();
        auto err = hipFree(static_cast<void *>(buf));
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
      });
}

template <class T>
inline typename _Unique_if<T>::_Unknown_bound make_unique_with_attributes(int hip_device,
                                                                          unsigned int flags,
                                                                          size_t n)
{
  using Tp = typename std::remove_all_extents<T>::type;
  return typename _Unique_if<T>::_Unknown_bound(
      [&]() {
        Tp *buf;
        int old;
        auto err = hipGetDevice(&old);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }

        err = hipSetDevice(hip_device);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
        err = hipExtMallocWithFlags(reinterpret_cast<void **>(&buf), sizeof(Tp) * n, flags);
        if (err != hipSuccess) {
          auto fail_msg = hipGetErrorString(err);
          err = hipSetDevice(old);
          if (err != hipSuccess) {
            auto setdev_fail_msg = hipGetErrorString(err);
            throw StatusError(Status::Code::NO_FREE_RESOURCE,
                              (std::string(fail_msg) + std::string(", ") +
                               std::string(setdev_fail_msg))
                                  .c_str());
          }
          throw StatusError(Status::Code::NO_FREE_RESOURCE, hipGetErrorString(err));
        }
        err = hipSetDevice(old);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }

        if constexpr (std::is_constructible<Tp>::value) {
          for (size_t i = 0; i < n; i++) {
            new (&buf[i]) Tp();
          }
        } else {
          for (size_t i = 0; i < n; i++) {
            new (&buf[i]) Tp{};
          }
        }
        return buf;
      }(),
      [n](Tp *buf) {
        if constexpr (std::is_destructible<Tp>::value) {
          for (auto i = 0; i < n; i++) {
            (&buf[i])->~Tp();
          }
        }
        auto err = hipFree(static_cast<void *>(buf));
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
      });
}

template <class T, class... Args>
inline typename _Unique_if<T>::_Single_object make_unique(int hip_device, Args &&...args)
{
  return make_unique_with_attributes<T, Args...>(hip_device, hipDeviceMallocDefault,
                                                 std::forward<Args &&>(args)...);
}

template <class T>
inline typename _Unique_if<T>::_Unknown_bound make_unique(int hip_device, size_t n)
{
  return make_unique_with_attributes<T>(hip_device, hipDeviceMallocDefault, n);
}

template <class T>
inline typename _Unique_if<T>::_Single_object allocate_unique_with_attributes(int hip_device,
                                                                              unsigned int flags)
{
  return typename _Unique_if<T>::_Single_object(
      [&]() {
        T *buf;
        int old;
        auto err = hipGetDevice(&old);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }

        err = hipSetDevice(hip_device);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
        err = hipExtMallocWithFlags(reinterpret_cast<void **>(&buf), sizeof(T), flags);
        if (err != hipSuccess) {
          auto fail_msg = hipGetErrorString(err);
          err = hipSetDevice(old);
          if (err != hipSuccess) {
            auto setdev_fail_msg = hipGetErrorString(err);
            throw StatusError(Status::Code::NO_FREE_RESOURCE,
                              (std::string(fail_msg) + std::string(", ") +
                               std::string(setdev_fail_msg))
                                  .c_str());
          }
          throw StatusError(Status::Code::NO_FREE_RESOURCE, hipGetErrorString(err));
        }
        err = hipSetDevice(old);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
        return buf;
      }(),
      [](T *buf) {
        auto err = hipFree(static_cast<void *>(buf));
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
      });
}

template <class T>
inline typename _Unique_if<T>::_Single_object allocate_unique(int hip_device)
{
  return allocate_unique_with_attributes<T>(hip_device, hipDeviceMallocDefault);
}

template <class T>
inline typename _Unique_if<T>::_Unknown_bound allocate_unique_with_attributes(int hip_device,
                                                                              unsigned int flags,
                                                                              size_t n)
{
  using Tp = typename std::remove_all_extents<T>::type;
  return typename _Unique_if<T>::_Unknown_bound(
      [&]() {
        Tp *buf;
        int old;
        auto err = hipGetDevice(&old);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }

        err = hipSetDevice(hip_device);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
        err = hipExtMallocWithFlags(reinterpret_cast<void **>(&buf), sizeof(Tp) * n, flags);
        if (err != hipSuccess) {
          auto fail_msg = hipGetErrorString(err);
          err = hipSetDevice(old);
          if (err != hipSuccess) {
            auto setdev_fail_msg = hipGetErrorString(err);
            throw StatusError(Status::Code::NO_FREE_RESOURCE,
                              (std::string(fail_msg) + std::string(", ") +
                               std::string(setdev_fail_msg))
                                  .c_str());
          }
          throw StatusError(Status::Code::NO_FREE_RESOURCE, hipGetErrorString(err));
        }
        err = hipSetDevice(old);
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
        return buf;
      }(),
      [](Tp *buf) {
        auto err = hipFree(static_cast<void *>(buf));
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
      });
}

template <class T>
inline typename _Unique_if<T>::_Unknown_bound allocate_unique(int hip_dev, size_t n)
{
  return allocate_unique_with_attributes<T>(hip_dev, hipDeviceMallocDefault, n);
}
///! @endcond

}  // namespace device
namespace host
{

///! @cond
template <class T, class... Args>
inline typename _Unique_if<T>::_Single_object make_unique_with_attributes(unsigned int flags,
                                                                          Args &&...args)
{
  return typename _Unique_if<T>::_Single_object(
      [&]() {
        T *buf;
        auto err = hipHostMalloc(reinterpret_cast<void **>(&buf), sizeof(T), flags);
        if (err != hipSuccess) {
          throw StatusError(Status::Code::NO_FREE_RESOURCE, hipGetErrorString(err));
        }
        if constexpr (std::is_constructible<T, Args...>::value) {
          new (buf) T(std::forward<Args &&>(args)...);
        } else {
          new (buf) T{std::forward<Args &&>(args)...};
        }
        return buf;
      }(),
      [](T *buf) {
        buf->~T();
        auto err = hipHostFree(static_cast<void *>(buf));
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
      });
}

template <class T>
inline typename _Unique_if<T>::_Unknown_bound make_unique_with_attributes(unsigned int flags,
                                                                          size_t n)
{
  using Tp = typename std::remove_all_extents<T>::type;
  return typename _Unique_if<T>::_Unknown_bound(
      [&]() {
        Tp *buf;
        auto err = hipHostMalloc(reinterpret_cast<void **>(&buf), sizeof(Tp) * n, flags);
        if (err != hipSuccess) {
          throw StatusError(Status::Code::NO_FREE_RESOURCE, hipGetErrorString(err));
        }

        if constexpr (std::is_constructible<Tp>::value) {
          for (size_t i = 0; i < n; i++) {
            new (&buf[i]) Tp();
          }
        } else {
          for (size_t i = 0; i < n; i++) {
            new (&buf[i]) Tp{};
          }
        }
        return buf;
      }(),
      [n](Tp *buf) {
        if constexpr (std::is_destructible<Tp>::value) {
          for (auto i = 0; i < n; i++) {
            (&buf[i])->~Tp();
          }
        }
        auto err = hipHostFree(static_cast<void *>(buf));
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
      });
}

template <class T, class... Args>
inline typename _Unique_if<T>::_Single_object make_unique(Args &&...args)
{
  return make_unique_with_attributes<T, Args...>(hipHostMallocDefault,
                                                 std::forward<Args &&>(args)...);
}

template <class T>
inline typename _Unique_if<T>::_Unknown_bound make_unique(size_t n)
{
  return make_unique_with_attributes<T>(hipHostMallocDefault, n);
}

template <class T>
inline typename _Unique_if<T>::_Single_object allocate_unique_with_attributes(unsigned int flags)
{
  return typename _Unique_if<T>::_Single_object(
      [&]() {
        T *buf;
        auto err = hipHostMalloc(reinterpret_cast<void **>(&buf), sizeof(T), flags);
        if (err != hipSuccess) {
          throw StatusError(Status::Code::NO_FREE_RESOURCE, hipGetErrorString(err));
        }
        return buf;
      }(),
      [](T *buf) {
        auto err = hipHostFree(static_cast<void *>(buf));
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
      });
}

template <class T>
inline typename _Unique_if<T>::_Single_object allocate_unique()
{
  return allocate_unique_with_attributes<T>(hipHostMallocDefault);
}

template <class T>
inline typename _Unique_if<T>::_Unknown_bound allocate_unique_with_attributes(unsigned int flags,
                                                                              size_t n)
{
  using Tp = typename std::remove_all_extents<T>::type;
  return typename _Unique_if<T>::_Unknown_bound(
      [&]() {
        Tp *buf;
        auto err = hipHostMalloc(reinterpret_cast<void **>(&buf), sizeof(Tp) * n, flags);
        if (err != hipSuccess) {
          throw StatusError(Status::Code::NO_FREE_RESOURCE, hipGetErrorString(err));
        }
        return buf;
      }(),
      [](Tp *buf) {
        auto err = hipHostFree(static_cast<void *>(buf));
        if (err != hipSuccess) {
          throw std::runtime_error(hipGetErrorString(err));
        }
      });
}

template <class T>
inline typename _Unique_if<T>::_Unknown_bound allocate_unique(size_t n)
{
  return allocate_unique_with_attributes<T>(hipHostMallocDefault, n);
}
///! @endcond

}  // namespace host
}  // namespace embers
#endif  // _EMBERS_FIRESTORM_DETAIL_MEMORY_H_
