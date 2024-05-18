cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_library(Runner STATIC
  runner.cpp
  rsvg-convert.cpp
)

target_link_libraries(Runner IO)

find_package(CLI11 CONFIG REQUIRED)
find_package(Boost 1.80.0 CONFIG REQUIRED COMPONENTS filesystem)

target_link_libraries(Runner CLI11::CLI11)
target_link_libraries(Runner Boost::boost Boost::filesystem)

target_compile_features(Runner PRIVATE cxx_std_20)

target_include_directories(Runner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
