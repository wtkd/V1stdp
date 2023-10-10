cmake_minimum_required(VERSION 3.20)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if (NOT CMAKE_CONFIGURATION_TYPES AND
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE AND
    CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "[!] Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(Filesystem STATIC
  filesystem.cpp
)

target_link_libraries(Filesystem IO)

find_package(CLI11 CONFIG REQUIRED)

target_link_libraries(Filesystem CLI11::CLI11)

target_compile_features(Filesystem PRIVATE cxx_std_20)

target_include_directories(Filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
