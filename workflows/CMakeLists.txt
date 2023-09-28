cmake_minimum_required(VERSION 3.20)

find_program(CCWL_EXECUTABLE NAMES "ccwl")
if (NOT CCWL_EXECUTABLE)
  message(WARNING
    "If you would like to generate common workflows language files, \"ccwl\", Concise Common Workflow Language, is needed. See https://ccwl.systemreboot.net/ and https://github.com/arunisaac/ccwl."
  )
endif()

find_program(GNUPLOT_EXECUTABLE NAMES "gnuplot")
if (NOT GNUPLOT_EXECUTABLE)
  message(WARNING
    "If you would like to run workflows, \"gnuplot\" is needed. See http://www.gnuplot.info/."
  )
endif()

# TODO: DEPENDS does not works well, because OUTPUT file is expected to be in binary dir, but it is in source directory

add_custom_command(
  OUTPUT learn.cwl
  COMMAND ${CCWL_EXECUTABLE} compile learn.scm > learn.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS learn.scm
  VERBATIM)
add_custom_target(generate_learn_cwl DEPENDS learn.cwl)

add_custom_command(
  OUTPUT test.cwl
  COMMAND ${CCWL_EXECUTABLE} compile test.scm > test.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS test.scm
  VERBATIM)
add_custom_target(generate_test_cwl DEPENDS test.cwl)

add_custom_command(
  OUTPUT simulation.cwl
  COMMAND ${CCWL_EXECUTABLE} compile simulation.scm > simulation.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS simulation.scm learn.cwl test.cwl
  VERBATIM)
add_custom_target(generate_simulation_cwl DEPENDS simulation.cwl)

add_custom_command(
  OUTPUT analyze.cwl
  COMMAND ${CCWL_EXECUTABLE} compile analyze.scm > analyze.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS analyze.scm
  VERBATIM)
add_custom_target(generate_analyze_cwl DEPENDS analyze.cwl)

add_custom_command(
  OUTPUT exportImages.cwl
  COMMAND ${CCWL_EXECUTABLE} compile exportImages.scm > exportImages.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS exportImages.scm
  VERBATIM)
add_custom_target(generate_exportImages_cwl DEPENDS exportImages.cwl)

add_custom_command(
  OUTPUT all.cwl
  COMMAND ${CCWL_EXECUTABLE} compile all.scm > all.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS all.scm learn.cwl test.cwl analyze.cwl exportImages.cwl
  VERBATIM)
add_custom_target(generate_all_cwl DEPENDS all.cwl)

add_custom_target(GenerateCWL DEPENDS generate_test_cwl generate_learn_cwl generate_simulation_cwl generate_analyze_cwl generate_exportImages_cwl generate_all_cwl)
