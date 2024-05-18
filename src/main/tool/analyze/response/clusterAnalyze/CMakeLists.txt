cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_library(ClusterAnalyze STATIC
  clusterAnalyze.cpp
)

target_link_libraries(ClusterAnalyze PUBLIC IO)

find_package(CLI11 CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(ClusterAnalyze PUBLIC CLI11::CLI11)
target_link_libraries(ClusterAnalyze PUBLIC Eigen3::Eigen)

target_compile_features(ClusterAnalyze PRIVATE cxx_std_20)

target_include_directories(ClusterAnalyze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
