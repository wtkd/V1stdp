cmake_minimum_required(VERSION 3.12)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if (NOT CMAKE_CONFIGURATION_TYPES AND
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE AND
    CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "[!] Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(Analyze STATIC
  analyze.cpp
  clustering.cpp)

find_package(CLI11 CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)

target_link_libraries(Analyze PUBLIC CLI11::CLI11)
target_link_libraries(Analyze PUBLIC Eigen3::Eigen)

target_compile_features(Analyze PRIVATE cxx_std_20)
