/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_UNIQUE_PTR_H_
#define _EMBERS_UNIQUE_PTR_H_

#include <functional>
#include <memory>

#include <hip/hip_runtime.h>

namespace embers
{

/// @brief smart pointer with __device__ code getters
///
/// Behaves identically to std::unique_ptr for host code.
/// Kernel code holds no ownership over unique_ptr
/// unique_ptr lifetime is managed by host code only
template <typename T,
          typename D = std::function<void(typename std::remove_all_extents<T>::type *)> >
class unique_ptr
{
 public:
  using pointer = typename std::unique_ptr<T, D>::pointer;
  using element_type = typename std::unique_ptr<T, D>::element_type;
  using deleter_type = typename std::unique_ptr<T, D>::deleter_type;

 private:
  std::tuple<pointer, std::unique_ptr<T, D> > m_;

  constexpr pointer &_P_ptr() noexcept { return std::get<0>(m_); }
  constexpr pointer _P_ptr() const noexcept { return std::get<0>(m_); }
  constexpr std::unique_ptr<T, D> &_U_ptr() noexcept { return std::get<1>(m_); }
  constexpr const std::unique_ptr<T, D> &_U_ptr() const noexcept { return std::get<1>(m_); }

 public:
  constexpr unique_ptr() noexcept : m_(std::make_tuple(nullptr, std::unique_ptr<T, D>(nullptr))) {}
  constexpr unique_ptr(std::nullptr_t) noexcept
      : m_(std::make_tuple(nullptr, std::unique_ptr<T, D>(nullptr)))
  {
  }
  explicit unique_ptr(pointer p) noexcept : m_(std::make_tuple(p, std::unique_ptr<T, D>(p))) {}

  unique_ptr(pointer p, D d1) noexcept : m_(std::make_tuple(p, std::unique_ptr<T, D>(p, d1))) {}
  unique_ptr(unique_ptr &&u) noexcept : m_(std::move(u.m_)) {}

  void swap(unique_ptr &other) noexcept { std::swap(m_, other.m_); }

  explicit unique_ptr(std::unique_ptr<T, D> &&p) noexcept
      : m_(std::make_tuple(nullptr, std::unique_ptr<T, D>()))
  {
    this->swap(p);
  }

  template <class U, class E>
  unique_ptr(unique_ptr<U, E> &&u) noexcept : m_(std::make_tuple(nullptr, std::unique_ptr<T, D>()))
  {
    this->swap(u);
  }

  unique_ptr &operator=(unique_ptr &&r) noexcept
  {
    this->swap(r);
    return *this;
  }

  template <class U, class E>
  unique_ptr &operator=(unique_ptr<U, E> &&r) noexcept
  {
    this->swap(r);
    return *this;
  }

  unique_ptr &operator=(std::nullptr_t) noexcept
  {
    this->reset(nullptr);
    return *this;
  }

  pointer release() noexcept
  {
    pointer old_ptr = _U_ptr().release();
    _P_ptr() = _U_ptr().get();
    return old_ptr;
  }

  void reset(pointer ptr = pointer()) noexcept
  {
    _U_ptr().reset(ptr);
    _P_ptr() = _U_ptr().get();
  }

  pointer get() const noexcept { return _U_ptr().get(); }

  __device__ pointer get() const noexcept { return _P_ptr(); }

  D &get_deleter() noexcept { return _U_ptr().get_deleter(); }

  const D &get_deleter() const noexcept { return _U_ptr().get_deleter(); }

  explicit operator bool() const noexcept { return static_cast<bool>(_U_ptr()); }

  __device__ explicit operator bool() const noexcept { return static_cast<bool>(_P_ptr()); }

  typename std::add_lvalue_reference<T>::type operator*() const
      noexcept(noexcept(*std::declval<pointer>()))
  {
    return *(_U_ptr());
  }

  __device__ typename std::add_lvalue_reference<T>::type operator*() const noexcept
  {
    return *(_P_ptr());
  }

  pointer operator->() const noexcept { return _U_ptr().get(); }

  __device__ pointer operator->() const noexcept { return _P_ptr(); }
};

template <class T1, class D1, class T2, class D2>
inline bool operator==(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() == y.get();
}

template <class T1, class D1, class T2, class D2>
inline bool operator<(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() < y.get();
}

template <class T1, class D1, class T2, class D2>
inline bool operator<=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() <= y.get();
}

template <class T1, class D1, class T2, class D2>
inline bool operator>(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() > y.get();
}

template <class T1, class D1, class T2, class D2>
inline bool operator>=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() >= y.get();
}

template <class T, class D>
inline bool operator==(const unique_ptr<T, D> &x, std::nullptr_t) noexcept
{
  return x.get() == std::nullptr_t();
}

template <class T, class D>
inline bool operator<(const unique_ptr<T, D> &x, std::nullptr_t)
{
  return x.get() < std::nullptr_t();
}
template <class T, class D>
inline bool operator<(std::nullptr_t, const unique_ptr<T, D> &y)
{
  return std::nullptr_t() < y.get();
}

template <class T, class D>
inline bool operator<=(const unique_ptr<T, D> &x, std::nullptr_t)
{
  return x.get() <= std::nullptr_t();
}

template <class T, class D>
inline bool operator<=(std::nullptr_t, const unique_ptr<T, D> &y)
{
  return std::nullptr_t() <= y.get();
}

template <class T, class D>
inline bool operator>(const unique_ptr<T, D> &x, std::nullptr_t)
{
  return x.get() > std::nullptr_t();
}

template <class T, class D>
inline bool operator>(std::nullptr_t, const unique_ptr<T, D> &y)
{
  return std::nullptr_t() > y._U_ptr();
}

template <class T, class D>
inline bool operator>=(const unique_ptr<T, D> &x, std::nullptr_t)
{
  return x._U_ptr() >= std::nullptr_t();
}

template <class T, class D>
inline bool operator>=(std::nullptr_t, const unique_ptr<T, D> &y)
{
  return std::nullptr_t() >= y._U_ptr();
}

template <class T1, class D1, class T2, class D2>
__device__ inline bool operator==(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() == y.get();
}

template <class T1, class D1, class T2, class D2>
__device__ inline bool operator<(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() < y.get();
}

template <class T1, class D1, class T2, class D2>
__device__ inline bool operator<=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() <= y.get();
}

template <class T1, class D1, class T2, class D2>
__device__ inline bool operator>(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() > y.get();
}

template <class T1, class D1, class T2, class D2>
__device__ inline bool operator>=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
  return x.get() >= y.get();
}

template <class T, class D>
__device__ inline bool operator==(const unique_ptr<T, D> &x, std::nullptr_t) noexcept
{
  return x.get() == std::nullptr_t();
}

template <class T, class D>
__device__ inline bool operator<(const unique_ptr<T, D> &x, std::nullptr_t)
{
  return x.get() < std::nullptr_t();
}
template <class T, class D>
__device__ inline bool operator<(std::nullptr_t, const unique_ptr<T, D> &y)
{
  return std::nullptr_t() < y.get();
}

template <class T, class D>
__device__ inline bool operator<=(const unique_ptr<T, D> &x, std::nullptr_t)
{
  return x.get() <= std::nullptr_t();
}

template <class T, class D>
__device__ inline bool operator<=(std::nullptr_t, const unique_ptr<T, D> &y)
{
  return std::nullptr_t() <= y.get();
}

template <class T, class D>
__device__ inline bool operator>(const unique_ptr<T, D> &x, std::nullptr_t)
{
  return x.get() > std::nullptr_t();
}

template <class T, class D>
__device__ inline bool operator>(std::nullptr_t, const unique_ptr<T, D> &y)
{
  return std::nullptr_t() > y.get();
}

template <class T, class D>
__device__ inline bool operator>=(const unique_ptr<T, D> &x, std::nullptr_t)
{
  return x.get() >= std::nullptr_t();
}

template <class T, class D>
__device__ inline bool operator>=(std::nullptr_t, const unique_ptr<T, D> &y)
{
  return std::nullptr_t() >= y.get();
}

template <typename T, typename D>
class unique_ptr<T[], D>
{
 public:
  using pointer = typename std::unique_ptr<T[], D>::pointer;
  using element_type = typename std::unique_ptr<T[], D>::element_type;
  using deleter_type = typename std::unique_ptr<T[], D>::deleter_type;

 private:
  std::tuple<pointer, std::unique_ptr<T[], D> > m_;

  constexpr pointer &_P_ptr() noexcept { return std::get<0>(m_); }
  constexpr pointer _P_ptr() const noexcept { return std::get<0>(m_); }
  constexpr std::unique_ptr<T[], D> &_U_ptr() noexcept { return std::get<1>(m_); }
  constexpr const std::unique_ptr<T[], D> &_U_ptr() const noexcept { return std::get<1>(m_); }

 public:
  constexpr unique_ptr() noexcept : m_(std::make_tuple(std::nullptr_t(), std::unique_ptr<T[]>())) {}
  constexpr unique_ptr(std::nullptr_t) noexcept
      : m_(std::make_tuple(std::nullptr_t(), std::unique_ptr<T[]>()))
  {
  }

  template <class U>
  explicit unique_ptr(U p) noexcept : m_(std::make_tuple(p, std::unique_ptr<T[]>(p)))
  {
  }
  unique_ptr(unique_ptr &&u) noexcept : m_(std::move(u.m_)) {}

  template <class U>
  unique_ptr(U p, D d1) noexcept : m_(std::make_tuple(p, std::unique_ptr<T[], D>(p, d1)))
  {
  }

  void swap(unique_ptr &other) noexcept { std::swap(m_, other.m_); }

  template <class U, class E>
  unique_ptr(unique_ptr<U, E> &&u) noexcept
      : m_(std::make_tuple(nullptr, std::unique_ptr<T[], D>()))
  {
    this->swap(u);
  }

  __host__ __device__ T &operator[](std::size_t i) const { return this->get()[i]; }

  unique_ptr &operator=(unique_ptr &&r) noexcept
  {
    this->swap(r);
    return *this;
  }

  template <class U, class E>
  unique_ptr &operator=(unique_ptr<U, E> &&r) noexcept
  {
    this->swap(r);
    return *this;
  }

  unique_ptr &operator=(std::nullptr_t) noexcept
  {
    this->reset(nullptr);
    return *this;
  }

  pointer release() noexcept
  {
    pointer old_ptr = _U_ptr().release();
    _P_ptr() = _U_ptr().get();
    return old_ptr;
  }

  void reset(pointer ptr = pointer()) noexcept
  {
    _U_ptr().reset(ptr);
    _P_ptr() = _U_ptr().get();
  }

  pointer get() const noexcept { return _U_ptr().get(); }

  __device__ pointer get() const noexcept { return _P_ptr(); }

  D &get_deleter() noexcept { return _U_ptr().get_deleter(); }

  const D &get_deleter() const noexcept { return _U_ptr().get_deleter(); }

  explicit operator bool() const noexcept { return static_cast<bool>(_U_ptr()); }

  __device__ explicit operator bool() const noexcept { return static_cast<bool>(_P_ptr()); }

  typename std::add_lvalue_reference<T>::type operator*() const
      noexcept(noexcept(*std::declval<pointer>()))
  {
    return *(_U_ptr());
  }

  __device__ typename std::add_lvalue_reference<T>::type operator*() const noexcept
  {
    return *(_P_ptr());
  }

  pointer operator->() const noexcept { return _U_ptr().get(); }

  __device__ pointer operator->() const noexcept { return _P_ptr; }
};

template <class T>
struct _Unique_if {
  typedef unique_ptr<T> _Single_object;
};

template <class T>
struct _Unique_if<T[]> {
  typedef unique_ptr<T[]> _Unknown_bound;
};

template <class T, size_t N>
struct _Unique_if<T[N]> {
  typedef void _Known_bound;
};

}  // namespace embers

namespace std
{
template <class T, class D>
void swap(embers::unique_ptr<T, D> &lhs, embers::unique_ptr<T, D> &rhs) noexcept
{
  lhs.swap(rhs);
}

template <class T, class Deleter>
struct hash<embers::unique_ptr<T, Deleter> > {
  constexpr decltype(auto) operator()(embers::unique_ptr<T, Deleter> x) const noexcept
  {
    return hash<decltype(x.get())>{}(x.get());
  }
};

}  // namespace std
#endif  // _EMBERS_UNIQUE_PTR_H_
