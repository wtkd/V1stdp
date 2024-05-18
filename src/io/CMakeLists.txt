cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_library(IO STATIC
  io.cpp
)

target_compile_features(IO PRIVATE cxx_std_20)

find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(IO Eigen3::Eigen)
target_link_libraries(IO Boost::boost)

target_include_directories(IO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
