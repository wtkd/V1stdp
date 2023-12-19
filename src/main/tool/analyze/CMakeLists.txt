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

add_subdirectory(applyPermutation)
add_subdirectory(delay)
add_subdirectory(divideLine)
add_subdirectory(response)
add_subdirectory(weight)
add_subdirectory(exploreMaximum)
add_subdirectory(reverseCorrelation)
add_subdirectory(transpose)

add_library(Analyze STATIC
  analyze.cpp
)

target_link_libraries(Analyze PUBLIC ApplyPermutation)
target_link_libraries(Analyze PUBLIC Delay)
target_link_libraries(Analyze PUBLIC DivideLine)
target_link_libraries(Analyze PUBLIC Response)
target_link_libraries(Analyze PUBLIC Weight)
target_link_libraries(Analyze PUBLIC ExploreMaximum)
target_link_libraries(Analyze PUBLIC reverseCorrelation)
target_link_libraries(Analyze PUBLIC Transpose)

find_package(CLI11 CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(Analyze PUBLIC CLI11::CLI11)
target_link_libraries(Analyze PUBLIC Eigen3::Eigen)
target_link_libraries(Analyze PUBLIC Boost::boost)

target_compile_features(Analyze PRIVATE cxx_std_20)

target_include_directories(Analyze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
