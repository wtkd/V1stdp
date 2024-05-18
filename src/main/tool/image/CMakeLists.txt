cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_library(Image STATIC
  image.cpp
  export.cpp
  exporter.cpp)

target_link_libraries(Image IO)

find_package(CLI11 CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)

target_link_libraries(Image CLI11::CLI11)
target_link_libraries(Image Boost::boost)
target_link_libraries(Image Eigen3::Eigen)

target_compile_features(Image PRIVATE cxx_std_20)

target_include_directories(Image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
