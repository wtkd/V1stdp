cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_subdirectory(config)
add_subdirectory(io)
add_subdirectory(statistics)
add_subdirectory(main)
