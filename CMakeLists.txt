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

add_subdirectory(src)

enable_testing()

# Ensure directory

add_test(
  NAME remove_data_directory
  COMMAND rm -rf ./data
)
set_tests_properties(
  remove_data_directory PROPERTIES
  FIXTURES_SETUP data_directory
)

add_test(
  NAME ensure_data_directory
  COMMAND mkdir ./data
)
set_tests_properties(
  ensure_data_directory PROPERTIES
  FIXTURES_SETUP data_directory
  DEPENDS remove_data_directory
)

# Learn

add_test(
  NAME run_learn
  COMMAND $<TARGET_FILE:stdp> learn --save-directory ./data --input-directory ${PROJECT_SOURCE_DIR} --seed 0 --step-number-learning 1000 --save-log-interval 500
)
set_tests_properties(
  run_learn PROPERTIES
  FIXTURES_SETUP run_learn
  FIXTURES_REQUIRED data_directory
  LABELS learn
)

add_test(
  NAME compare_weight
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w.txt ./data/w.txt
)
set_tests_properties(
  compare_weight PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_dat
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w.dat ./data/w.dat
)
set_tests_properties(
  compare_weight_dat PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff.txt ./data/wff.txt
)
set_tests_properties(
  compare_weight_feedforward PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_dat
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff.dat ./data/wff.dat
)
set_tests_properties(
  compare_weight_feedforward_dat PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_response
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/resps.txt ./data/resps.txt
)
set_tests_properties(
  compare_response PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_0
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w_0.txt ./data/w_0.txt
)
set_tests_properties(
  compare_weight_0 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_0_data
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w_0.dat ./data/w_0.dat
)
set_tests_properties(
  compare_weight_0_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_0
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff_0.txt ./data/wff_0.txt
)
set_tests_properties(
  compare_weight_feedforward_0 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_0_data
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff_0.dat ./data/wff_0.dat
)
set_tests_properties(
  compare_weight_feedforward_0_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_500
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w_500.txt ./data/w_500.txt
)
set_tests_properties(
  compare_weight_500 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_500_data
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w_500.dat ./data/w_500.dat
)
set_tests_properties(
  compare_weight_500_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_500
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff_500.txt ./data/wff_500.txt
)
set_tests_properties(
  compare_weight_feedforward_500 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_500_data
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff_500.dat ./data/wff_500.dat
)
set_tests_properties(
  compare_weight_feedforward_500_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_1000
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w_1000.txt ./data/w_1000.txt
)
set_tests_properties(
  compare_weight_1000 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_1000_data
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/w_1000.dat ./data/w_1000.dat
)
set_tests_properties(
  compare_weight_1000_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_1000
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff_1000.txt ./data/wff_1000.txt
)
set_tests_properties(
  compare_weight_feedforward_1000 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_1000_data
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/wff_1000.dat ./data/wff_1000.dat
)
set_tests_properties(
  compare_weight_feedforward_1000_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

# Test

add_test(
  NAME run_test
  COMMAND $<TARGET_FILE:stdp> test --save-directory ./data --load-directory ${CMAKE_SOURCE_DIR}/test/data/ --input-directory ${PROJECT_SOURCE_DIR} --seed 0 --step-number-testing 1000
)
set_tests_properties(
  run_test PROPERTIES
  FIXTURES_SETUP run_test
  FIXTURES_REQUIRED data_directory
  LABELS test
)

add_test(
  NAME compare_lastnspikes_test
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/lastnspikes_test.txt ./data/lastnspikes_test.txt
)
set_tests_properties(
  compare_lastnspikes_test PROPERTIES
  FIXTURES_REQUIRED run_test
  LABELS test
)

add_test(
  NAME compare_response_test
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/resps_test.txt ./data/resps_test.txt
)
set_tests_properties(
  compare_response_test PROPERTIES
  FIXTURES_REQUIRED run_test
  LABELS test
)

add_test(
  NAME compare_lastnv_test
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/lastnv_test.txt ./data/lastnv_test.txt
)
set_tests_properties(
  compare_lastnv_test PROPERTIES
  FIXTURES_REQUIRED run_test
  LABELS test
)

# Pulse

add_test(
  NAME run_pulse
  COMMAND $<TARGET_FILE:stdp> pulse --save-directory ./data --load-directory ${CMAKE_SOURCE_DIR}/test/data/ --input-directory ${PROJECT_SOURCE_DIR} --seed 0 50
)
set_tests_properties(
  run_pulse PROPERTIES
  FIXTURES_SETUP run_pulse
  FIXTURES_REQUIRED data_directory
  LABELS pulse
)

add_test(
  NAME compare_lastnspikes_pulse
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/lastnspikes_pulse.txt ./data/lastnspikes_pulse.txt
)
set_tests_properties(
  compare_lastnspikes_pulse PROPERTIES
  FIXTURES_REQUIRED run_pulse
  LABELS pulse
)

add_test(
  NAME compare_response_pulse
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/resps_pulse.txt ./data/resps_pulse.txt
)
set_tests_properties(
  compare_response_pulse PROPERTIES
  FIXTURES_REQUIRED run_pulse
  LABELS pulse
)

# Spontaneous

add_test(
  NAME run_spontaneous
  COMMAND $<TARGET_FILE:stdp> spontaneous --save-directory ./data --load-directory ${CMAKE_SOURCE_DIR}/test/data/ --input-directory ${PROJECT_SOURCE_DIR} --seed 0
)
set_tests_properties(
  run_spontaneous PROPERTIES
  FIXTURES_SETUP run_spontaneous
  FIXTURES_REQUIRED data_directory
  LABELS spontaneous
)

add_test(
  NAME compare_lastnspikes_spont
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/lastnspikes_spont.txt ./data/lastnspikes_spont.txt
)
set_tests_properties(
  compare_lastnspikes_spont PROPERTIES
  FIXTURES_REQUIRED run_spontaneous
  LABELS spontaneous
)

# Mix

add_test(
  NAME run_mix
  COMMAND $<TARGET_FILE:stdp> mix --save-directory ./data --load-directory ${CMAKE_SOURCE_DIR}/test/data/ --input-directory ${PROJECT_SOURCE_DIR} --seed 0 50 100
)
set_tests_properties(
  run_mix PROPERTIES
  FIXTURES_SETUP run_mix
  FIXTURES_REQUIRED data_directory
  LABELS mix
)

add_test(
  NAME compare_response_mix
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/resps_mix.txt ./data/resps_mix.txt
)
set_tests_properties(
  compare_response_mix PROPERTIES
  FIXTURES_REQUIRED run_mix
  LABELS mix
)

add_test(
  NAME compare_respssumv_mix
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/respssumv_mix.txt ./data/respssumv_mix.txt
)
set_tests_properties(
  compare_respssumv_mix PROPERTIES
  FIXTURES_REQUIRED run_mix
  LABELS mix
)

# Tool

## Analyze

### Response

#### Clustering

##### All neuron

add_test(
  NAME run_clustering_all
  COMMAND $<TARGET_FILE:stdp> tool analyze response clustering ${CMAKE_SOURCE_DIR}/test/data/resps_test.txt -o ./data/sorted_response_all.txt --stimulation ./data/sort_index_stimulation_all.txt --neuron ./data/sort_index_neuron_all.txt --excitatory-neuron-number 100 --inhibitory-neuron-number 20
)
set_tests_properties(
  run_clustering_all PROPERTIES
  FIXTURES_SETUP run_clustering_all
  FIXTURES_REQUIRED data_directory
  LABELS clustering
)

add_test(
  NAME compare_sorted_response_all
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sorted_response_all.txt ./data/sorted_response_all.txt
)
set_tests_properties(
  compare_sorted_response_all PROPERTIES
  FIXTURES_REQUIRED run_clustering_all
  LABELS clustering
)

add_test(
  NAME compare_sort_index_neuron_all
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_all.txt ./data/sort_index_neuron_all.txt
)
set_tests_properties(
  compare_sort_index_neuron_all PROPERTIES
  FIXTURES_REQUIRED run_clustering_all
  LABELS clustering
)

add_test(
  NAME compare_sort_index_stimulation_all
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_stimulation_all.txt ./data/sort_index_stimulation_all.txt
)
set_tests_properties(
  compare_sort_index_stimulation_all PROPERTIES
  FIXTURES_REQUIRED run_clustering_all
  LABELS clustering
)

##### Excitatory only

add_test(
  NAME run_clustering_excitatory_only
  COMMAND $<TARGET_FILE:stdp> tool analyze response clustering ${CMAKE_SOURCE_DIR}/test/data/resps_test.txt -o ./data/sorted_response_excitatory_only.txt --stimulation ./data/sort_index_stimulation_excitatory_only.txt --neuron ./data/sort_index_neuron_excitatory_only.txt --excitatory-neuron-number 100 --inhibitory-neuron-number 20 --excitatory-only
)
set_tests_properties(
  run_clustering_excitatory_only PROPERTIES
  FIXTURES_SETUP run_clustering_excitatory_only
  FIXTURES_REQUIRED data_directory
  LABELS clustering
)

add_test(
  NAME compare_sorted_response_excitatory_only
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sorted_response_excitatory_only.txt ./data/sorted_response_excitatory_only.txt
)
set_tests_properties(
  compare_sorted_response_excitatory_only PROPERTIES
  FIXTURES_REQUIRED run_clustering_excitatory_only
  LABELS clustering
)

add_test(
  NAME compare_sort_index_neuron_excitatory_only
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_excitatory_only.txt ./data/sort_index_neuron_excitatory_only.txt
)
set_tests_properties(
  compare_sort_index_neuron_excitatory_only PROPERTIES
  FIXTURES_REQUIRED run_clustering_excitatory_only
  LABELS clustering
)

add_test(
  NAME compare_sort_index_stimulation_excitatory_only
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_stimulation_excitatory_only.txt ./data/sort_index_stimulation_excitatory_only.txt
)
set_tests_properties(
  compare_sort_index_stimulation_excitatory_only PROPERTIES
  FIXTURES_REQUIRED run_clustering_excitatory_only
  LABELS clustering
)

##### Inhibitory only

add_test(
  NAME run_clustering_inhibitory_only
  COMMAND $<TARGET_FILE:stdp> tool analyze response clustering ${CMAKE_SOURCE_DIR}/test/data/resps_test.txt -o ./data/sorted_response_inhibitory_only.txt --stimulation ./data/sort_index_stimulation_inhibitory_only.txt --neuron ./data/sort_index_neuron_inhibitory_only.txt --excitatory-neuron-number 100 --inhibitory-neuron-number 20 --inhibitory-only
)
set_tests_properties(
  run_clustering_inhibitory_only PROPERTIES
  FIXTURES_SETUP run_clustering_inhibitory_only
  FIXTURES_REQUIRED data_directory
  LABELS clustering
)

add_test(
  NAME compare_sorted_response_inhibitory_only
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sorted_response_inhibitory_only.txt ./data/sorted_response_inhibitory_only.txt
)
set_tests_properties(
  compare_sorted_response_inhibitory_only PROPERTIES
  FIXTURES_REQUIRED run_clustering_inhibitory_only
  LABELS clustering
)

add_test(
  NAME compare_sort_index_neuron_inhibitory_only
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_inhibitory_only.txt ./data/sort_index_neuron_inhibitory_only.txt
)
set_tests_properties(
  compare_sort_index_neuron_inhibitory_only PROPERTIES
  FIXTURES_REQUIRED run_clustering_inhibitory_only
  LABELS clustering
)

add_test(
  NAME compare_sort_index_stimulation_inhibitory_only
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_stimulation_inhibitory_only.txt ./data/sort_index_stimulation_inhibitory_only.txt
)
set_tests_properties(
  compare_sort_index_stimulation_inhibitory_only PROPERTIES
  FIXTURES_REQUIRED run_clustering_inhibitory_only
  LABELS clustering
)

#### Apply permutation

##### Both row and colomn

add_test(
  NAME run_apply_permutation_both
  COMMAND $<TARGET_FILE:stdp> tool analyze apply-permutation ${CMAKE_SOURCE_DIR}/test/data/correlationMatrix_neuron.txt --colomn ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_all.txt --row ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_all.txt ./data/correlationMatrix_neuron_permutated_both.txt
)
set_tests_properties(
  run_apply_permutation_both PROPERTIES
  FIXTURES_SETUP run_apply_permutation
  FIXTURES_REQUIRED data_directory
  LABELS apply_permutation
)

add_test(
  NAME compare_apply_permutation_both
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/correlationMatrix_neuron_permutated_both.txt ./data/correlationMatrix_neuron_permutated_both.txt
)
set_tests_properties(
  compare_apply_permutation_both PROPERTIES
  FIXTURES_REQUIRED run_apply_permutation_both
  LABELS apply_permutation
)

##### Row

add_test(
  NAME run_apply_permutation_row
  COMMAND $<TARGET_FILE:stdp> tool analyze apply-permutation ${CMAKE_SOURCE_DIR}/test/data/correlationMatrix_neuron.txt --colomn ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_all.txt ./data/correlationMatrix_neuron_permutated_row.txt
)
set_tests_properties(
  run_apply_permutation_row PROPERTIES
  FIXTURES_SETUP run_apply_permutation
  FIXTURES_REQUIRED data_directory
  LABELS apply_permutation
)

add_test(
  NAME compare_apply_permutation_row
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/correlationMatrix_neuron_permutated_row.txt ./data/correlationMatrix_neuron_permutated_row.txt
)
set_tests_properties(
  compare_apply_permutation_row PROPERTIES
  FIXTURES_REQUIRED run_apply_permutation_row
  LABELS apply_permutation
)

add_test(
  NAME run_apply_permutation_colomn
  COMMAND $<TARGET_FILE:stdp> tool analyze apply-permutation ${CMAKE_SOURCE_DIR}/test/data/correlationMatrix_neuron.txt --colomn ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_all.txt ./data/correlationMatrix_neuron_permutated_colomn.txt
)
set_tests_properties(
  run_apply_permutation_colomn PROPERTIES
  FIXTURES_SETUP run_apply_permutation
  FIXTURES_REQUIRED data_directory
  LABELS apply_permutation
)

add_test(
  NAME compare_apply_permutation_colomn
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/correlationMatrix_neuron_permutated_colomn.txt ./data/correlationMatrix_neuron_permutated_colomn.txt
)
set_tests_properties(
  compare_apply_permutation_colomn PROPERTIES
  FIXTURES_REQUIRED run_apply_permutation_colomn
  LABELS apply_permutation
)

#### Cut

##### Excitatory only

add_test(
  NAME run_response_cut_excitatory
  COMMAND $<TARGET_FILE:stdp> tool analyze response cut ${CMAKE_SOURCE_DIR}/test/data/resps_test.txt --excitatory-neuron-number 100 --inhibitory-neuron-number 20 --stimulation-number 1000 --excitatory-only-output ./data/response_excitatory.txt
)
set_tests_properties(
  run_response_cut_excitatory PROPERTIES
  FIXTURES_SETUP run_response_cut_excitatory
  FIXTURES_REQUIRED data_directory
  LABELS response_cut
)

add_test(
  NAME compare_response_excitatory
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/response_excitatory.txt ./data/response_excitatory.txt
)
set_tests_properties(
  compare_response_excitatory PROPERTIES
  FIXTURES_REQUIRED run_response_cut_excitatory
  LABELS response_cut
)

##### Inhibitory only

add_test(
  NAME run_response_cut_inhibitory
  COMMAND $<TARGET_FILE:stdp> tool analyze response cut ${CMAKE_SOURCE_DIR}/test/data/resps_test.txt --excitatory-neuron-number 100 --inhibitory-neuron-number 20 --stimulation-number 1000 --inhibitory-only-output ./data/response_inhibitory.txt
)
set_tests_properties(
  run_response_cut_inhibitory PROPERTIES
  FIXTURES_SETUP run_response_cut_inhibitory
  FIXTURES_REQUIRED data_directory
  LABELS response_cut
)

add_test(
  NAME compare_response_inhibitory
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/response_inhibitory.txt ./data/response_inhibitory.txt
)
set_tests_properties(
  compare_response_inhibitory PROPERTIES
  FIXTURES_REQUIRED run_response_cut_inhibitory
  LABELS response_cut
)

##### Run clustering with cut response should output same result

###### Excitatory only

add_test(
  NAME run_clustering_all_with_cut_excitatory
  COMMAND $<TARGET_FILE:stdp> tool analyze response clustering ${CMAKE_SOURCE_DIR}/test/data/response_excitatory.txt -o ./data/sorted_response_all_with_cut_excitatory.txt --stimulation ./data/sort_index_stimulation_all_with_cut_excitatory.txt --neuron ./data/sort_index_neuron_all_with_cut_excitatory.txt --excitatory-neuron-number 100 --inhibitory-neuron-number 0
)
set_tests_properties(
  run_clustering_all_with_cut_excitatory PROPERTIES
  FIXTURES_SETUP run_clustering_all_with_cut_excitatory
  FIXTURES_REQUIRED data_directory
  LABELS "clustering;response_cut"
)

add_test(
  NAME compare_sorted_response_all_with_cut_excitatory
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sorted_response_excitatory_only.txt ./data/sorted_response_all_with_cut_excitatory.txt
)
set_tests_properties(
  compare_sorted_response_all_with_cut_excitatory PROPERTIES
  FIXTURES_REQUIRED run_clustering_all_with_cut_excitatory
  LABELS "clustering;response_cut"
)

add_test(
  NAME compare_sort_index_neuron_all_with_cut_excitatory
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_excitatory_only.txt ./data/sort_index_neuron_all_with_cut_excitatory.txt
)
set_tests_properties(
  compare_sort_index_neuron_all_with_cut_excitatory PROPERTIES
  FIXTURES_REQUIRED run_clustering_all_with_cut_excitatory
  LABELS "clustering;response_cut"
)

add_test(
  NAME compare_sort_index_stimulation_all_with_cut_excitatory
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_stimulation_excitatory_only.txt ./data/sort_index_stimulation_all_with_cut_excitatory.txt
)
set_tests_properties(
  compare_sort_index_stimulation_all_with_cut_excitatory PROPERTIES
  FIXTURES_REQUIRED run_clustering_all_with_cut_excitatory
  LABELS "clustering;response_cut"
)

###### Inhibitory only

add_test(
  NAME run_clustering_all_with_inhibitory
  COMMAND $<TARGET_FILE:stdp> tool analyze response clustering ${CMAKE_SOURCE_DIR}/test/data/response_inhibitory.txt -o ./data/sorted_response_all_with_inhibitory.txt --stimulation ./data/sort_index_stimulation_all_with_inhibitory.txt --neuron ./data/sort_index_neuron_all_with_inhibitory.txt --excitatory-neuron-number 0 --inhibitory-neuron-number 20
)
set_tests_properties(
  run_clustering_all_with_inhibitory PROPERTIES
  FIXTURES_SETUP run_clustering_all_with_inhibitory
  FIXTURES_REQUIRED data_directory
  LABELS "clustering;response_cut"
)

add_test(
  NAME compare_sorted_response_all_with_cut_inhibitory
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sorted_response_inhibitory_only.txt ./data/sorted_response_all_with_inhibitory.txt
)
set_tests_properties(
  compare_sorted_response_all_with_cut_inhibitory PROPERTIES
  FIXTURES_REQUIRED run_clustering_all_with_inhibitory
  LABELS "clustering;response_cut"
)

add_test(
  NAME compare_sort_index_neuron_all_with_cut_inhibitory
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_neuron_inhibitory_only.txt ./data/sort_index_neuron_all_with_inhibitory.txt
)
set_tests_properties(
  compare_sort_index_neuron_all_with_cut_inhibitory PROPERTIES
  FIXTURES_REQUIRED run_clustering_all_with_inhibitory
  LABELS "clustering;response_cut"
)

add_test(
  NAME compare_sort_index_stimulation_all_with_cut_inhibitory
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/sort_index_stimulation_inhibitory_only.txt ./data/sort_index_stimulation_all_with_inhibitory.txt
)
set_tests_properties(
  compare_sort_index_stimulation_all_with_cut_inhibitory PROPERTIES
  FIXTURES_REQUIRED run_clustering_all_with_inhibitory
  LABELS "clustering;response_cut"
)

#### Correlation matrix

add_test(
  NAME run_correlation_matrix
  COMMAND $<TARGET_FILE:stdp> tool analyze response correlation-matrix ${CMAKE_SOURCE_DIR}/test/data/resps_test.txt --neuron-number 120 --stimulation-number 1000 -n ./data/correlationMatrix_neuron.txt -s ./data/correlationMatrix_stimulation.txt
)
set_tests_properties(
  run_correlation_matrix PROPERTIES
  FIXTURES_SETUP run_correlation_matrix
  FIXTURES_REQUIRED data_directory
  LABELS correlation_matrix
)

add_test(
  NAME compare_correlation_matrix_neuron
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/correlationMatrix_neuron.txt ./data/correlationMatrix_neuron.txt
)
set_tests_properties(
  compare_correlation_matrix_neuron PROPERTIES
  FIXTURES_REQUIRED run_correlation_matrix
  LABELS correlation_matrix
)

add_test(
  NAME compare_correlation_matrix_stimulation
  COMMAND cmp ${CMAKE_SOURCE_DIR}/test/data/correlationMatrix_stimulation.txt ./data/correlationMatrix_stimulation.txt
)
set_tests_properties(
  compare_correlation_matrix_stimulation PROPERTIES
  FIXTURES_REQUIRED run_correlation_matrix
  LABELS correlation_matrix
)
