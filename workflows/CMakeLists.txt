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

find_program(RSVG_CONVERT_EXECUTABLE NAMES "rsvg-convert")
if (NOT RSVG_CONVERT_EXECUTABLE)
  message(WARNING
    "If you would like to run workflows, \"rsvg-convert\" is needed. See https://wiki.gnome.org/Projects/LibRsvg."
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
  DEPENDS simulation.scm generate_learn_cwl generate_test_cwl
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
  DEPENDS all.scm generate_learn_cwl generate_test_cwl generate_analyze_cwl generate_exportImages_cwl
  VERBATIM)
add_custom_target(generate_all_cwl DEPENDS all.cwl)

add_custom_command(
  OUTPUT responseDivide.cwl
  COMMAND ${CCWL_EXECUTABLE} compile responseDivide.scm > responseDivide.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS responseDivide.scm
  VERBATIM)
add_custom_target(generate_responseDivide_cwl DEPENDS responseDivide.cwl)

add_custom_command(
  OUTPUT run-explore-maximum.cwl
  COMMAND ${CCWL_EXECUTABLE} compile run-explore-maximum.scm > run-explore-maximum.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS run-explore-maximum.scm
  VERBATIM)
add_custom_target(generate_run-explore-maximum_cwl DEPENDS run-explore-maximum.cwl)

add_custom_command(
  OUTPUT analyze-explore-maximum.cwl
  COMMAND ${CCWL_EXECUTABLE} compile analyze-explore-maximum.scm > analyze-explore-maximum.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS analyze-explore-maximum.scm
  VERBATIM)
add_custom_target(generate_analyze-explore-maximum_cwl DEPENDS analyze-explore-maximum.cwl)

add_custom_command(
  OUTPUT all-explore-maximum.cwl
  COMMAND ${CCWL_EXECUTABLE} compile all-explore-maximum.scm > all-explore-maximum.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS all-explore-maximum.scm generate_run-explore-maximum_cwl generate_analyze-explore-maximum_cwl
  VERBATIM)
add_custom_target(generate_all-explore-maximum_cwl DEPENDS all-explore-maximum.cwl)

add_custom_command(
  OUTPUT run-explore-maximum-with-noise.cwl
  COMMAND ${CCWL_EXECUTABLE} compile run-explore-maximum-with-noise.scm > run-explore-maximum-with-noise.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS run-explore-maximum-with-noise.scm
  VERBATIM)
add_custom_target(generate_run-explore-maximum-with-noise_cwl DEPENDS run-explore-maximum-with-noise.cwl)

add_custom_command(
  OUTPUT all-explore-maximum-with-noise.cwl
  COMMAND ${CCWL_EXECUTABLE} compile all-explore-maximum-with-noise.scm > all-explore-maximum-with-noise.cwl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS all-explore-maximum-with-noise.scm generate_run-explore-maximum-with-noise_cwl generate_analyze-explore-maximum_cwl
  VERBATIM)
add_custom_target(generate_all-explore-maximum-with-noise_cwl DEPENDS all-explore-maximum-with-noise.cwl)

add_custom_target(GenerateCWL DEPENDS generate_test_cwl generate_learn_cwl generate_simulation_cwl generate_analyze_cwl generate_exportImages_cwl generate_responseDivide_cwl generate_all_cwl generate_run-explore-maximum_cwl generate_analyze-explore-maximum_cwl generate_all-explore-maximum_cwl generate_run-explore-maximum-with-noise_cwl generate_all-explore-maximum-with-noise_cwl)
