/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <cstdint>
#include <cstring>
#include <future>
#include <limits>

#include <hip/hip_runtime.h>

#include "embers/status.h"
#include "embers/memory.h"
#include "embers/rand/fill_buffer_rand.h"
#include "embers/rand/xorshift.cuh"

#include "test_helpers.h"

using namespace embers;

template <typename T>
__global__ void FillBufferKernel(T *ptr, size_t num_elems, rand::xorshift128p_state *state, T a,
                                 T b)
{
  rand::FillBufferRandom(ptr, num_elems, state, a, b);
}
// Generate 2 buffers each on host and device.
// Fill the buffers with FillBufferRandom.
// Consistency check the host buffers and then check the device buffers.
// The device buffers are copied back to host for consistency checking the host
// against the device data.
template <typename T>
void TestFillBufferRand(T low, T high)
{
  if (high == std::numeric_limits<T>::max()) {
    throw StatusError(Status::Code::CODE_BUG, "high must be below maximum value for the type");
  }
  constexpr size_t num_elems = 1024;
  constexpr uint32_t block_size = 64;
  constexpr uint32_t num_blocks = num_elems / block_size;

  int gpu;
  HIP_CHECK(hipGetDevice(&gpu));

  std::array<hipStream_t, 2> gpu_streams;
  for (auto &strm : gpu_streams) {
    HIP_CHECK(hipStreamCreate(&strm));
  }
  std::array<hipStream_t, 2> cpu_streams;
  for (auto &strm : cpu_streams) {
    HIP_CHECK(hipStreamCreate(&strm));
  }

  auto cpu_state = host::allocate_unique<rand::xorshift128p_state>();
  rand::xorshift128p_init(cpu_state.get(), 1337);

  auto gpu_state = device::allocate_unique<rand::xorshift128p_state>(gpu);
  HIP_CHECK(hipMemcpyHtoDAsync(gpu_state.get(), cpu_state.get(), sizeof(rand::xorshift128p_state),
                               gpu_streams.at(0)));

  auto initial_data = host::allocate_unique<T[]>(num_elems);

  // 2 buffers for consistency check
  std::array host_data = {host::allocate_unique<T[]>(num_elems),
                          host::allocate_unique<T[]>(num_elems)};
  std::array dev_data = {device::allocate_unique<T[]>(gpu, num_elems),
                         device::allocate_unique<T[]>(gpu, num_elems)};
  constexpr unsigned int num_buffers = host_data.size();

  // Initialize the data to a value > high.
  // This is to ensure that the kernel actually writes to the buffer.
  for (size_t i = 0; i < num_elems; i++) {
    initial_data[i] = std::numeric_limits<T>::max();
  }

  // Copy initial data and overwrite the buffers with random data.
  std::vector<std::future<void>> cpu_futures(num_buffers);
  for (unsigned int i = 0; i < num_buffers; i++) {
    HIP_CHECK(hipMemcpyHtoDAsync(dev_data.at(i).get(), initial_data.get(), num_elems * sizeof(T),
                                 gpu_streams.at(i)));

    hipLaunchKernelGGL(FillBufferKernel<T>, dim3(num_blocks), dim3(block_size), 0,
                       gpu_streams.at(i), dev_data.at(i).get(), num_elems, gpu_state.get(), low,
                       high);
    cpu_futures.at(
        i) = std::async(std::launch::async, [&initial_data, &host_data, i, &cpu_state, low, high] {
      std::memcpy(host_data.at(i).get(), initial_data.get(), num_elems * sizeof(T));
      rand::FillBufferRandom<T>(host_data.at(i).get(), num_elems, cpu_state.get(), low, high);
    });
  }
  for (auto &strm : gpu_streams) {
    HIP_CHECK(hipStreamSynchronize(strm));
  }
  for (auto &future : cpu_futures) {
    future.get();
  }

  auto assert_in_range = [low, high](const T *data) {
    for (size_t i = 0; i < num_elems; i++) {
      if (data[i] < low || data[i] > high) {
        throw StatusError(Status::Code::ERROR, "Generated value outside of required range");
      }
    }
  };

  constexpr auto assert_array_equal = [](const T *a, const T *b) {
    if (std::memcmp(a, b, num_elems * sizeof(T)) != 0) {
      throw StatusError(Status::Code::ERROR,
                           "Host buffers are inconsistent. Generated different "
                           "data for the same seed!");
    }
  };

  // Check CPU data
  for (unsigned int i = 0; i < num_buffers; i++) {
    assert_in_range(host_data.at(i).get());
  }
  assert_array_equal(host_data.at(0).get(), host_data.at(1).get());

  // Reuse CPU buffers to check GPU data
  for (unsigned int i = 0; i < num_buffers; i++) {
    HIP_CHECK(hipMemcpyDtoHAsync(host_data.at(i).get(), dev_data.at(i).get(), num_elems * sizeof(T),
                                 gpu_streams.at(i)));
  }
  for (auto &strm : gpu_streams) {
    HIP_CHECK(hipStreamSynchronize(strm));
  }
  for (unsigned int i = 0; i < num_buffers; i++) {
    assert_in_range(host_data.at(i).get());
  }
  assert_array_equal(host_data.at(0).get(), host_data.at(1).get());

  for (auto strm : cpu_streams) {
    HIP_CHECK(hipStreamDestroy(strm));
  }
  for (auto strm : gpu_streams) {
    HIP_CHECK(hipStreamDestroy(strm));
  }
}

int main()
{
  TestFillBufferRand<uint32_t>(0, 100);
  TestFillBufferRand<int>(-50000, 50000);
  TestFillBufferRand<double>(-1.0, 1.0);
}
