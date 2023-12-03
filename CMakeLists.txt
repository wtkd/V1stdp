## ENABLE_LINT=ON if you want to run linters

cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(stdp CXX)

if (NOT CMAKE_CONFIGURATION_TYPES AND
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE AND
    CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "[!] Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif()

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(src)
add_subdirectory(workflows)

enable_testing()

# Unit tests
if (NOT NOUNITTEST)
  message(STATUS
    "Unit test is turned on. It needs GoogleTest. You can turn it off by passing \"-DNOUNITTEST=True\"."
  )

  find_package(GTest REQUIRED)

  add_executable(
    stdp_unit_test
    test/unit-test/main.cpp
  )

  target_link_libraries(
    stdp_unit_test
    GTest::gtest_main
  )

  target_compile_features(stdp_unit_test PRIVATE cxx_std_20)

  gtest_discover_tests(
    stdp_unit_test
    PROPERTIES LABELS unit_test
  )
endif()

# Integration tests
include(test/test.cmake)
