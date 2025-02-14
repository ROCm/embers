/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <stdio.h>
#include <string.h>
#include "embers/status.h"

using namespace embers;

int main()
{
  if (Status().StatusCode() != Status::Code::SUCCESS) {
    return 1;
  }
  if (bool(Status(Status::Code::SUCCESS))) {
    return 1;
  }

  auto code = Status::Code::ERROR;
  auto msg = std::string("blah an error occured");
  auto s = Status(code, msg);
  if (s.StatusCode() != code) {
    return 1;
  };
  if (s.Message().find(msg) == std::string::npos) {
    return 1;
  };

  auto s_no_ref = Status(Status::Code::ERROR, "this has no ref");
  if (s_no_ref.StatusCode() != Status::Code::ERROR) {
    return 1;
  }
  if (s_no_ref.Message().find("this has no ref") == std::string::npos) {
    return 1;
  }

  auto s_string = Status(Status::Code::ERROR, std::string("this is a string"));
  if (s_string.StatusCode() != Status::Code::ERROR) {
    return 1;
  }
  if (s_string.Message().find("this is a string") == std::string::npos) {
    return 1;
  }

  int num = 5;
  auto t = ValueOrError<int>(num);
  if (!bool(t)) {
    return 1;
  }
  if (!(*t == num)) {
    return 1;
  };

  auto s2 = ValueOrError<int>(Status(Status::Code::ERROR));
  if (bool(s2)) {
    return 1;
  };
  if (s2.Err().StatusCode() != Status::Code::ERROR) {
    return 1;
  };

  bool took_except = false;
  try {
    throw StatusError(Status(Status::Code::ERROR));

  } catch (StatusError &e) {
    took_except = true;
    auto s = e.GetStatus();
    if (!bool(s)) {
      return 1;
    };
    if (s.StatusCode() != Status::Code::ERROR) {
      return 1;
    };
    if (strcmp(s.Message().c_str(), e.what()) != 0) {
      return 1;
    };
  }
  if (!took_except) {
    return 1;
  }

  // make sure StatusError cannot be built with a successful Error
  took_except = false;
  try {
    auto j = StatusError(Status());
  } catch (std::runtime_error &e) {
    took_except = true;
  }
  if (!took_except) {
    return 1;
  };

  return 0;
}
