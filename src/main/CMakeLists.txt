cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_executable(stdp
  stdp.cpp)

add_subdirectory(simulation)
target_link_libraries(stdp Simulation)

add_subdirectory(tool)
target_link_libraries(stdp Tool)

target_link_libraries(stdp config)

find_package(Eigen3 REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(stdp Eigen3::Eigen)
target_link_libraries(stdp CLI11::CLI11)
target_link_libraries(stdp Boost::boost)

target_compile_features(stdp PRIVATE cxx_std_20)
