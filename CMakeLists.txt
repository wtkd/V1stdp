cmake_minimum_required(VERSION 3.12)

project(stdp CXX)

if (NOT CMAKE_CONFIGURATION_TYPES AND
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE AND
    CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "[!] Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(stdp
  stdp.cpp
  )

find_package(Eigen3 REQUIRED)
find_package(cxxopts REQUIRED)

target_link_libraries(stdp Eigen3::Eigen)
target_link_libraries(stdp cxxopts::cxxopts)

target_compile_features(stdp PRIVATE cxx_std_20)
