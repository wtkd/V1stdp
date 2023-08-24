cmake_minimum_required(VERSION 3.12)

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

add_library(Response STATIC
  response.cpp
  clustering.cpp
  correlationMatrix.cpp
  cut.cpp
)

target_link_libraries(Response PUBLIC IO)
target_link_libraries(Response PUBLIC Statistics)

find_package(CLI11 CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(Response PUBLIC CLI11::CLI11)
target_link_libraries(Response PUBLIC Eigen3::Eigen)
target_link_libraries(Response PUBLIC Boost::boost)

target_compile_features(Response PRIVATE cxx_std_20)

target_include_directories(Response PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
