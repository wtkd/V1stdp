cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_library(config INTERFACE
  config.hpp.in)

# Configure VERSION constant.
execute_process(
  COMMAND git describe
  OUTPUT_VARIABLE VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
configure_file(config.hpp.in config.hpp)

target_include_directories(config INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
