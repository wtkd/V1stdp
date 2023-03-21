cmake_minimum_required(VERSION 3.12)

# Inhibit in-source build
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(stdp CXX)

if (NOT CMAKE_CONFIGURATION_TYPES AND
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE AND
    CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "[!] Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(stdp
  stdp.cpp
  )

find_package(Eigen3 REQUIRED)
find_package(cxxopts REQUIRED)

target_link_libraries(stdp Eigen3::Eigen)
target_link_libraries(stdp cxxopts::cxxopts)

target_compile_features(stdp PRIVATE cxx_std_20)

# Configure VERSION constant.
execute_process(
  COMMAND git describe
  OUTPUT_VARIABLE VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
configure_file(config.hpp.in config.hpp)
target_include_directories(
  stdp PUBLIC
  "${PROJECT_BINARY_DIR}"
)

enable_testing()

add_test(
  NAME remove_directory
  COMMAND rm -rf ./data && mkdir ./data
)

add_test(
  NAME ensure_directory
  COMMAND mkdir ./data
)

add_test(
  NAME run_learn
  COMMAND $<TARGET_FILE:stdp> learn --data-directory ./data --input-directory .. --seed 0 --step-number-learning 1000
)
set_tests_properties(
  run_learn PROPERTIES
  DEPENDS ensure_directory
)

add_test(
  NAME run_test
  COMMAND $<TARGET_FILE:stdp> test --data-directory ./data --input-directory .. --seed 0 --step-number-learning 100
)
set_tests_properties(
  run_test PROPERTIES
  DEPENDS run_learn
)

add_test(
  NAME compare_weight
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w.txt ./data/w.txt
)
set_tests_properties(
  compare_weight PROPERTIES
  DEPENDS run_learn
)

add_test(
  NAME compare_weight_dat
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w.dat ./data/w.dat
)
set_tests_properties(
  compare_weight PROPERTIES
  DEPENDS run_learn
)

add_test(
  NAME compare_weight_feedforward
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff.txt ./data/wff.txt
)
set_tests_properties(
  compare_weight_feedforward PROPERTIES
  DEPENDS run_learn
)

add_test(
  NAME compare_weight_feedforward_dat
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff.dat ./data/wff.dat
)
set_tests_properties(
  compare_weight_feedforward PROPERTIES
  DEPENDS run_learn
)

add_test(
  NAME compare_response
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/resps.txt ./data/resps.txt
)
set_tests_properties(
  compare_response PROPERTIES
  DEPENDS run_learn
)

add_test(
  NAME compare_lastnspikes
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/lastnspikes.txt ./data/lastnspikes.txt
)
set_tests_properties(
  compare_lastnspikes PROPERTIES
  DEPENDS run_learn
)

add_test(
  NAME compare_response_test
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/resps_test.txt ./data/resps_test.txt
)
set_tests_properties(
  compare_response_test PROPERTIES
  DEPENDS run_test
)

add_test(
  NAME compare_lastnv_test
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/lastnv_test.txt ./data/lastnv_test.txt
)
set_tests_properties(
  compare_lastnv_test PROPERTIES
  DEPENDS run_test
)
