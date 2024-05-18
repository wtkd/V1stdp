cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_library(SplitAdd STATIC
  splitAdd.cpp
)

target_link_libraries(SplitAdd PUBLIC IO)
target_link_libraries(SplitAdd PUBLIC Statistics)

find_package(Eigen3 REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

target_link_libraries(SplitAdd PUBLIC CLI11::CLI11)
target_link_libraries(SplitAdd PUBLIC Eigen3::Eigen)

target_compile_features(SplitAdd PRIVATE cxx_std_20)

target_include_directories(SplitAdd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
