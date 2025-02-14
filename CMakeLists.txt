# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved


# stop if cmake version below 3.10
cmake_minimum_required(VERSION 3.11.4 FATAL_ERROR)
set(EMBERS_VERSION 26.0.0)

project(
embers
VERSION ${EMBERS_VERSION}
DESCRIPTION "A header-only HIP library with a slew of user-focused features for GPU test development"
LANGUAGES CXX HIP
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(hip REQUIRED)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "relWithDebInfo" CACHE STRING 
      " Default ubild type: Release with Debug info" FORCE)
endif()


add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
  INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}}/include>
  $<INSTALL_INTERFACE:include/embers>
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)



# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  set(EMBERS_TEST_PKG_NAME "embers-tests")
  add_subdirectory(tests)
  if(NOT (${CMAKE_BUILD_TYPE} STREQUAL External))
    install(PROGRAMS utils/run_emberstests.sh
      DESTINATION bin COMPONENT tests
    )
  endif()
endif()


# create cmake configuration files
include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_NAME}-config.cmake.in
                              ${PROJECT_NAME}-config.cmake
                              INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
  COMPATIBILITY SameMajorVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  DESTINATION "lib/cmake/${PROJECT_NAME}"
  COMPONENT dev
)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# install public headers
install(DIRECTORY
  ${PROJECT_SOURCE_DIR}/include/embers
  DESTINATION include
  COMPONENT dev
)


include(${CMAKE_CURRENT_SOURCE_DIR}/Packaging.cmake)
