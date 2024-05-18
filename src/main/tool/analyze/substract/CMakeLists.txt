cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_library(Substract STATIC
  substract.cpp
)

target_link_libraries(Substract PUBLIC IO)
target_link_libraries(Substract PUBLIC Statistics)

find_package(Eigen3 REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

target_link_libraries(Substract PUBLIC CLI11::CLI11)
target_link_libraries(Substract PUBLIC Eigen3::Eigen)

target_compile_features(Substract PRIVATE cxx_std_20)

target_include_directories(Substract PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
