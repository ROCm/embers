/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _EMBERS_STATUS_H_
#define _EMBERS_STATUS_H_

#include <sstream>
#include <variant>
#include <functional>
#include <exception>

#include <hip/hip_runtime.h>

#include "embers/helpers/std_source_location.h"

namespace embers
{
class StatusError;  // forward declaration
struct Status {
  // Top nibble is the category, bottom nibble is status number
  enum class Code : int {
    SUCCESS = 0x00,
    ERROR = 0x01,
    NO_FREE_RESOURCE = 0x05,
    OUT_OF_RANGE = 0x06,
    UNEXPECTED_NULL_PTR = 0x08,
    ALIGNMENT_ERROR = 0x09,
    ACCESS_NOT_ALLOWED = 0x11,
    ORDERING_ERR = 0x20,
    GROUP_NOT_VALID = 0x21,
    CODE_BUG = 0x22,
    TIMEOUT = 0x26,
  };

  inline const char *StatusCodeAsString() const
  {
    switch (code_) {
      case Code::SUCCESS:
        return "SUCCESS";
      case Code::ERROR:
        return "SUCCESS";
      case Code::NO_FREE_RESOURCE:
        return "NO_FREE_RESOURCE";
      case Code::OUT_OF_RANGE:
        return "OUT_OF_RANGE";
      case Code::UNEXPECTED_NULL_PTR:
        return "UNEXPECTED_NULL_PTR";
      case Code::ALIGNMENT_ERROR:
        return "ALIGNMENT_ERROR";
      case Code::ACCESS_NOT_ALLOWED:
        return "ACCESS_NOT_ALLOWED";
      case Code::ORDERING_ERR:
        return "ORDERING_ERR";
      case Code::GROUP_NOT_VALID:
        return "GROUP_NOT_VALID";
      case Code::CODE_BUG:
        return "CODE_BUG";
      case Code::TIMEOUT:
        return "TIMEOUT";
      default:
        return "UNKNOWN";
    }
  }

  Status(Code code = Code::SUCCESS, std::string msg = "",
         const std_source_location location = std_source_location::current())
      : code_(code), msg_(msg)
  {
    if (HAVE_STD_SOURCE_LOCATION) {
      std::stringstream ss;
      ss << location.file_name() << ":" << location.line() << " `" << location.function_name()
         << "`";

      if (!msg_.empty()) {
        ss << " | ";
      }

      msg_.insert(0, ss.str());
    }
  }

  // Allow this to be tested for an error condition in an if statement
  explicit operator bool() const noexcept { return bool(code_); }

  // Allow comparison
  bool operator==(const Status &other) const noexcept { return code_ == other.code_; }
  bool operator!=(const Status &other) const noexcept { return code_ != other.code_; }

  // Pretty printing
  operator std::string() const noexcept
  {
    std::stringstream ss;

    if (msg_.size()) {
      ss << msg_ << " - ";
    }
    ss << "0x" << std::hex << static_cast<int>(code_) << " (" << StatusCodeAsString() << ")";
    return ss.str();
  }

  const std::string &Message() const noexcept { return msg_; }

  Code StatusCode() const noexcept { return code_; }

  void RaiseIfError() const;

  // Error code
  Code code_;

  // Optional error message
  std::string msg_;
};

template <typename T>
class ValueOrError
{
  std::variant<T, Status> m_v;

 public:
  ValueOrError(Status s) : m_v(std::move(s)) {}
  ValueOrError(T t) : m_v(std::move(t)) {}

  Status Err() const noexcept
  {
    if (auto e = std::get_if<Status>(&m_v)) {
      return *e;
    } else {
      return Status();
    }
  }

  T &operator*() { return std::get<0>(m_v); }
  T const &operator*() const noexcept { return std::get<0>(m_v); }
  explicit operator bool() const noexcept { return std::holds_alternative<T>(m_v); }
};

template <typename T>
class ValueOrError<T &>
{
  std::variant<std::reference_wrapper<T>, Status> m_v;

 public:
  ValueOrError(Status s) : m_v(std::move(s)) {}
  ValueOrError(T &t) : m_v(t) {}

  Status Err() const noexcept
  {
    if (auto e = std::get_if<Status>(&m_v)) {
      return *e;
    } else {
      return Status();
    }
  }

  T &operator*() { return std::get<0>(m_v); }
  T const &operator*() const noexcept { return std::get<0>(m_v); }
  explicit operator bool() const noexcept
  {
    return std::holds_alternative<std::reference_wrapper<T> >(m_v);
  }
};

class StatusError : public std::exception
{
 private:
  Status s_;

 public:
  StatusError(Status::Code code = Status::Code::ERROR, std::string msg = "",
              const std_source_location location = std_source_location::current())
      : s_(Status(code, msg, location))
  {
  }

  StatusError(Status s) : s_(s)
  {
    if (!s) {
      throw std::runtime_error(
          "Cannot create an StatusError with an Error with code Status::Code::SUCCESS!");
    }
  }

  const char *what() const noexcept { return s_.Message().c_str(); };

  Status GetStatus() const noexcept { return s_; };
};

inline void Status::RaiseIfError() const
{
  if (*this) {
    throw StatusError(*this);
  }
}

inline Status StatusFromHipError(
    hipError_t err, const std_source_location location = std_source_location::current())
{
  if (err == hipSuccess) {
    return Status();
  }
  const char *err_name = hipGetErrorName(err);
  const char *msg = hipGetErrorString(err);
  std::stringstream err_msg;
  err_msg << err_name << ":" << msg;
  return Status(Status::Code::ERROR, err_msg.str(), location);
}

}  // namespace embers
#endif  // _EMBERS_STATUS_H_
