cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_library(Statistics INTERFACE
  statistics.hpp
)

find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(Statistics INTERFACE Eigen3::Eigen)
target_link_libraries(Statistics INTERFACE Boost::boost)

target_include_directories(Statistics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
