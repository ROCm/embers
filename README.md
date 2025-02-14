<!-- Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved -->

<a name="readme-top"></a>

<!-- PROJECT LOGO
<br />
<div align="center">
  <a href="https://github.com/ROCm/embers">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>
-->

<h3 align="center">Embers</h3>

  <p align="center">
    A header-only HIP library with a slew of user-focused features for GPU test development
    <br />
    <a href="https://github.com/pages/ROCm/embers/index.html"<strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/ROCm/embers/issues/new?labels=bug&template=bug-report---.md">Report a Bug</a>
    ·
    <a href="https://github.com/ROCm/embers/issues/new?labels=enhancement&template=feature-request---.md">Request a Feature</a>
  </p>
</div>
<!-- ABOUT THE PROJECT -->

## About Embers

Embers is a header-only HIP library with a slew of features enhancing heterogeneous programming. These features are broken down into the following blocks:

- **amdgpu**
  - Functions providing GPU family based on PCI Device ID
  - Functionality to detect and report HW blocks running this code (SEs, CUs, waves)
- **crypto**
  - Implementations of keccak, cubehash, and ethash
- **helpers**
  - Templated format agnostic numeric equality checkers
  - Templated bit helpers
  - HIP kernels for memcpy, memset, and memcmp
  - Xlator class for translating system VAs to PAs
- **memory**
  - Heterogeneous unique_ptr with host side ownership
- **primitives**
  - Heterogeneous multi-device synchronization primitives
    - barriers
    - monotonic counters
    - locks
- **rand**
  - Multiple pseudorandom number generator implementations
  - Fill buffer rand heterogeneous helper

<p align="right"><a href="#readme-top">back to top</a></p>

### Built With

<div align="center">

<a href=""></a>[![ROCm™][rocm]][rocm-url]</a>
<a href=""></a>[![C++][C++]][C++-url]</a>
<a href=""></a>[![Cmake][Cmake]][Cmake-url]</a>
<a href=""></a>[![Linux][Linux]][Linux-url]</a>

</div>

<p align="right"><a href="#readme-top">back to top</a></p>

## Getting Started

### Build From Source

#### Prerequisites

- Cmake
- Compiler supporting HIP and C++20 (LLVM is a good choice)

#### Build

```sh
sudo apt install cmake
git clone --recursive https://github.com/ROCm/embers
cd embers && mkdir -p build && cd build
CXX=<path_to_hipcc> cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<path_to_rocm> ..
make -j
```

Either `make install` or `make -j` package and install the built packages

<p align="right"><a href="#readme-top">back to top</a></p>

## Usage

See the unit tests

<p align="right"><a href="#readme-top">back to top</a></p>

<!-- CONTRIBUTING -->

## Contributing

Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right"><a href="#readme-top">back to top</a></p>

<!-- LICENSE -->

## License

See `LICENSE.txt` for more information.

<p align="right"><a href="#readme-top">back to top</a></p>

<!-- CONTACT -->

## Contact

Project Link: [https://github.com/ROCm/embers](https://github.com/ROCm/embers)

<p align="right"><a href="#readme-top">back to top</a></p>

[rocm]: https://img.shields.io/badge/ROCm%E2%84%A2-grey?style=for-the-badge&logo=amd&labelColor=ED1C24
[rocm-url]: https://rocm.docs.amd.com
[C++]: https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white
[C++-url]: https://isocpp.org
[CMake]: https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white
[CMake-url]: https://cmake.org
[Linux]: https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black
[Linux-url]: https://www.linux.org
