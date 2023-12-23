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
  COMMAND $<TARGET_FILE:stdp> learn
  --save-directory ./data/learn
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --seed 0
  --step-number-learning 100
  --save-log-interval 50
  --image-range -200
  --start-learning-number 0
  --random-delay
)
set_tests_properties(
  run_learn PROPERTIES
  FIXTURES_SETUP run_learn
  FIXTURES_REQUIRED data_directory
  LABELS learn
)

add_test(
  NAME compare_weight
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/w.txt ./data/learn/w.txt
)
set_tests_properties(
  compare_weight PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_dat
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/w.dat ./data/learn/w.dat
)
set_tests_properties(
  compare_weight_dat PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/wff.txt ./data/learn/wff.txt
)
set_tests_properties(
  compare_weight_feedforward PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_dat
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/wff.dat ./data/learn/wff.dat
)
set_tests_properties(
  compare_weight_feedforward_dat PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_response
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/resps.txt ./data/learn/resps.txt
)
set_tests_properties(
  compare_response PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_0
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/w_0.txt ./data/learn/w_0.txt
)
set_tests_properties(
  compare_weight_0 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_0_data
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/w_0.dat ./data/learn/w_0.dat
)
set_tests_properties(
  compare_weight_0_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_0
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/wff_0.txt ./data/learn/wff_0.txt
)
set_tests_properties(
  compare_weight_feedforward_0 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_0_data
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/wff_0.dat ./data/learn/wff_0.dat
)
set_tests_properties(
  compare_weight_feedforward_0_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_50
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/w_50.txt ./data/learn/w_50.txt
)
set_tests_properties(
  compare_weight_50 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_50_data
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/w_50.dat ./data/learn/w_50.dat
)
set_tests_properties(
  compare_weight_50_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_50
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/wff_50.txt ./data/learn/wff_50.txt
)
set_tests_properties(
  compare_weight_feedforward_50 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_50_data
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/wff_50.dat ./data/learn/wff_50.dat
)
set_tests_properties(
  compare_weight_feedforward_50_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_100
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/w_100.txt ./data/learn/w_100.txt
)
set_tests_properties(
  compare_weight_100 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_100_data
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/w_100.dat ./data/learn/w_100.dat
)
set_tests_properties(
  compare_weight_100_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_100
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/wff_100.txt ./data/learn/wff_100.txt
)
set_tests_properties(
  compare_weight_feedforward_100 PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

add_test(
  NAME compare_weight_feedforward_100_data
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/learn/wff_100.dat ./data/learn/wff_100.dat
)
set_tests_properties(
  compare_weight_feedforward_100_data PROPERTIES
  FIXTURES_REQUIRED run_learn
  LABELS learn
)

# Test

## Without saved delays

add_test(
  NAME run_test_without_saved_delays
  COMMAND $<TARGET_FILE:stdp> test
  --save-directory ./data/without-saved-delays/test
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --seed 0
  --step-number-testing 150
  --image-range 200
  --random-delay
)
set_tests_properties(
  run_test_without_saved_delays PROPERTIES
  FIXTURES_SETUP run_test_without_saved_delays
  FIXTURES_REQUIRED data_directory
  LABELS test
)

add_test(
  NAME compare_lastnspikes_test_without_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/without-saved-delays/test/lastnspikes_test.txt ./data/without-saved-delays/test/lastnspikes_test.txt
)
set_tests_properties(
  compare_lastnspikes_test_without_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_test_without_saved_delays
  LABELS test
)

add_test(
  NAME compare_response_test_without_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/without-saved-delays/test/resps_test.txt ./data/without-saved-delays/test/resps_test.txt
)
set_tests_properties(
  compare_response_test_without_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_test_without_saved_delays
  LABELS test
)

add_test(
  NAME compare_lastnv_test_without_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/without-saved-delays/test/lastnv_test.txt ./data/without-saved-delays/test/lastnv_test.txt
)
set_tests_properties(
  compare_lastnv_test_without_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_test_without_saved_delays
  LABELS test
)

## With saved delays

add_test(
  NAME run_test_with_saved_delays
  COMMAND $<TARGET_FILE:stdp> test
  --save-directory ./data/with-saved-delays/test
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --seed 0
  --step-number-testing 150
  --image-range 200
  --delays-file ${CMAKE_SOURCE_DIR}/test/data/learn/delays.txt
)
set_tests_properties(
  run_test_with_saved_delays PROPERTIES
  FIXTURES_SETUP run_test_with_saved_delays
  FIXTURES_REQUIRED data_directory
  LABELS test
)

add_test(
  NAME compare_lastnspikes_test_with_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/with-saved-delays/test/lastnspikes_test.txt ./data/with-saved-delays/test/lastnspikes_test.txt
)
set_tests_properties(
  compare_lastnspikes_test_with_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_test_with_saved_delays
  LABELS test
)

add_test(
  NAME compare_response_test_with_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/with-saved-delays/test/resps_test.txt ./data/with-saved-delays/test/resps_test.txt
)
set_tests_properties(
  compare_response_test_with_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_test_with_saved_delays
  LABELS test
)

add_test(
  NAME compare_lastnv_test_with_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/with-saved-delays/test/lastnv_test.txt ./data/with-saved-delays/test/lastnv_test.txt
)
set_tests_properties(
  compare_lastnv_test_with_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_test_with_saved_delays
  LABELS test
)

# Pulse

## Without saved delays

add_test(
  NAME run_pulse_without_saved_delays
  COMMAND $<TARGET_FILE:stdp> pulse 50
  --save-directory ./data/without-saved-delays/pulse
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --seed 0
  --random-delay
)
set_tests_properties(
  run_pulse_without_saved_delays PROPERTIES
  FIXTURES_SETUP run_pulse_without_saved_delays
  FIXTURES_REQUIRED data_directory
  LABELS pulse
)

add_test(
  NAME compare_lastnspikes_pulse_without_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/without-saved-delays/pulse/lastnspikes_pulse_49.txt ./data/without-saved-delays/pulse/lastnspikes_pulse_49.txt
)
set_tests_properties(
  compare_lastnspikes_pulse_without_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_pulse_without_saved_delays
  LABELS pulse
)

add_test(
  NAME compare_response_pulse_without_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/without-saved-delays/pulse/resps_pulse_49.txt ./data/without-saved-delays/pulse/resps_pulse_49.txt
)
set_tests_properties(
  compare_response_pulse_without_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_pulse_without_saved_delays
  LABELS pulse
)

## With saved delays

add_test(
  NAME run_pulse_with_saved_delays
  COMMAND $<TARGET_FILE:stdp> pulse 50
  --save-directory ./data/with-saved-delays/pulse
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --seed 0
  --delays-file ${CMAKE_SOURCE_DIR}/test/data/learn/delays.txt
)
set_tests_properties(
  run_pulse_with_saved_delays PROPERTIES
  FIXTURES_SETUP run_pulse_with_saved_delays
  FIXTURES_REQUIRED data_directory
  LABELS pulse
)

add_test(
  NAME compare_lastnspikes_pulse_with_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/with-saved-delays/pulse/lastnspikes_pulse_49.txt ./data/with-saved-delays/pulse/lastnspikes_pulse_49.txt
)
set_tests_properties(
  compare_lastnspikes_pulse_with_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_pulse_with_saved_delays
  LABELS pulse
)

add_test(
  NAME compare_response_pulse_with_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/with-saved-delays/pulse/resps_pulse_49.txt ./data/with-saved-delays/pulse/resps_pulse_49.txt
)
set_tests_properties(
  compare_response_pulse_with_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_pulse_with_saved_delays
  LABELS pulse
)

# Spontaneous

## Without saved delays

add_test(
  NAME run_spontaneous_without_saved_delays
  COMMAND $<TARGET_FILE:stdp> spontaneous
  --save-directory ./data/without-saved-delays/spontaneous
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --seed 0
  --step 100
  --presentation-time 100
  --random-delay
)
set_tests_properties(
  run_spontaneous_without_saved_delays PROPERTIES
  FIXTURES_SETUP run_spontaneous_without_saved_delays
  FIXTURES_REQUIRED data_directory
  LABELS spontaneous
)

add_test(
  NAME compare_lastnspikes_spont_without_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/without-saved-delays/spontaneous/lastnspikes_spont.txt ./data/without-saved-delays/spontaneous/lastnspikes_spont.txt
)
set_tests_properties(
  compare_lastnspikes_spont_without_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_spontaneous_without_saved_delays
  LABELS spontaneous
)

## With saved delays

add_test(
  NAME run_spontaneous_with_saved_delays
  COMMAND $<TARGET_FILE:stdp> spontaneous
  --save-directory ./data/with-saved-delays/spontaneous
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --seed 0
  --step 100
  --presentation-time 100
  --delays-file ${CMAKE_SOURCE_DIR}/test/data/learn/delays.txt
)
set_tests_properties(
  run_spontaneous_with_saved_delays PROPERTIES
  FIXTURES_SETUP run_spontaneous_with_saved_delays
  FIXTURES_REQUIRED data_directory
  LABELS spontaneous
)

add_test(
  NAME compare_lastnspikes_spont_with_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/with-saved-delays/spontaneous/lastnspikes_spont.txt ./data/with-saved-delays/spontaneous/lastnspikes_spont.txt
)
set_tests_properties(
  compare_lastnspikes_spont_with_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_spontaneous_with_saved_delays
  LABELS spontaneous
)

# Mix

## Without saved delays

add_test(
  NAME run_mix_without_saved_delays
  COMMAND $<TARGET_FILE:stdp> mix 50 100
  --save-directory ./data/without-saved-delays/mix
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --seed 0
  --random-delay
)
set_tests_properties(
  run_mix_without_saved_delays PROPERTIES
  FIXTURES_SETUP run_mix_without_saved_delays
  FIXTURES_REQUIRED data_directory
  LABELS mix
)

add_test(
  NAME compare_response_mix_without_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/without-saved-delays/mix/resps_mix_49_99.txt ./data/without-saved-delays/mix/resps_mix_49_99.txt
)
set_tests_properties(
  compare_response_mix_without_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_mix_without_saved_delays
  LABELS mix
)

add_test(
  NAME compare_respssumv_mix_without_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/without-saved-delays/mix/respssumv_mix_49_99.txt ./data/without-saved-delays/mix/respssumv_mix_49_99.txt
)
set_tests_properties(
  compare_respssumv_mix_without_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_mix_without_saved_delays
  LABELS mix
)

## With saved delays

add_test(
  NAME run_mix_with_saved_delays
  COMMAND $<TARGET_FILE:stdp> mix 50 100
  --save-directory ./data/with-saved-delays/mix
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --seed 0
  --delays-file ${CMAKE_SOURCE_DIR}/test/data/learn/delays.txt
)
set_tests_properties(
  run_mix_with_saved_delays PROPERTIES
  FIXTURES_SETUP run_mix_with_saved_delays
  FIXTURES_REQUIRED data_directory
  LABELS mix
)

add_test(
  NAME compare_response_mix_with_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/with-saved-delays/mix/resps_mix_49_99.txt ./data/with-saved-delays/mix/resps_mix_49_99.txt
)
set_tests_properties(
  compare_response_mix_with_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_mix_with_saved_delays
  LABELS mix
)

add_test(
  NAME compare_respssumv_mix_with_saved_delays
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/with-saved-delays/mix/respssumv_mix_49_99.txt ./data/with-saved-delays/mix/respssumv_mix_49_99.txt
)
set_tests_properties(
  compare_respssumv_mix_with_saved_delays PROPERTIES
  FIXTURES_REQUIRED run_mix_with_saved_delays
  LABELS mix
)

# Tool

## Analyze

### Response

#### Clustering

##### Excitatory neuron

add_test(
  NAME run_clustering_excitatory
  COMMAND $<TARGET_FILE:stdp> tool analyze response clustering
  ${CMAKE_SOURCE_DIR}/test/data/analyze/response_excitatory.txt
  -o ./data/analyze/sorted_response_excitatory.txt
  --stimulation ./data/analyze/sort_index_stimulation_all.txt
  --neuron ./data/analyze/sort_index_neuron_excitatory.txt
  --neuron-number 100
  --stimulation-number 1000
)
set_tests_properties(
  run_clustering_excitatory PROPERTIES
  FIXTURES_SETUP run_clustering_excitatory
  FIXTURES_REQUIRED data_directory
  LABELS clustering
)

add_test(
  NAME compare_sorted_response_excitatory
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/sorted_response_excitatory.txt ./data/analyze/sorted_response_excitatory.txt
)
set_tests_properties(
  compare_sorted_response_excitatory PROPERTIES
  FIXTURES_REQUIRED run_clustering_excitatory
  LABELS clustering
)

add_test(
  NAME compare_sort_index_neuron_excitatory
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/sort_index_neuron_excitatory.txt ./data/analyze/sort_index_neuron_excitatory.txt
)
set_tests_properties(
  compare_sort_index_neuron_excitatory PROPERTIES
  FIXTURES_REQUIRED run_clustering_excitatory
  LABELS clustering
)

add_test(
  NAME compare_sort_index_stimulation_all
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/sort_index_stimulation_all.txt ./data/analyze/sort_index_stimulation_all.txt
)
set_tests_properties(
  compare_sort_index_stimulation_all PROPERTIES
  FIXTURES_REQUIRED run_clustering_excitatory
  LABELS clustering
)

#### Apply permutation

##### Both row and colomn

add_test(
  NAME run_apply_permutation_both
  COMMAND $<TARGET_FILE:stdp> tool analyze apply-permutation
  ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron.txt ./data/analyze/correlationMatrix_neuron_permutated_both.txt
  --colomn ${CMAKE_SOURCE_DIR}/test/data/analyze/sort_index_neuron_excitatory.txt
  --row ${CMAKE_SOURCE_DIR}/test/data/analyze/sort_index_neuron_excitatory.txt

)
set_tests_properties(
  run_apply_permutation_both PROPERTIES
  FIXTURES_SETUP run_apply_permutation_both
  FIXTURES_REQUIRED data_directory
  LABELS apply_permutation
)

add_test(
  NAME compare_apply_permutation_both
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron_permutated_both.txt ./data/analyze/correlationMatrix_neuron_permutated_both.txt
)
set_tests_properties(
  compare_apply_permutation_both PROPERTIES
  FIXTURES_REQUIRED run_apply_permutation_both
  LABELS apply_permutation
)

##### Row

add_test(
  NAME run_apply_permutation_row
  COMMAND $<TARGET_FILE:stdp> tool analyze apply-permutation
  ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron.txt ./data/analyze/correlationMatrix_neuron_permutated_row.txt
  --row ${CMAKE_SOURCE_DIR}/test/data/analyze/sort_index_neuron_excitatory.txt
)
set_tests_properties(
  run_apply_permutation_row PROPERTIES
  FIXTURES_SETUP run_apply_permutation_row
  FIXTURES_REQUIRED data_directory
  LABELS apply_permutation
)

add_test(
  NAME compare_apply_permutation_row
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron_permutated_row.txt ./data/analyze/correlationMatrix_neuron_permutated_row.txt
)
set_tests_properties(
  compare_apply_permutation_row PROPERTIES
  FIXTURES_REQUIRED run_apply_permutation_row
  LABELS apply_permutation
)

##### Colomn

add_test(
  NAME run_apply_permutation_colomn
  COMMAND $<TARGET_FILE:stdp> tool analyze apply-permutation
  ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron.txt ./data/analyze/correlationMatrix_neuron_permutated_colomn.txt
  --colomn ${CMAKE_SOURCE_DIR}/test/data/analyze/sort_index_neuron_excitatory.txt
)
set_tests_properties(
  run_apply_permutation_colomn PROPERTIES
  FIXTURES_SETUP run_apply_permutation_colomn
  FIXTURES_REQUIRED data_directory
  LABELS apply_permutation
)

add_test(
  NAME compare_apply_permutation_colomn
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron_permutated_colomn.txt ./data/analyze/correlationMatrix_neuron_permutated_colomn.txt
)
set_tests_properties(
  compare_apply_permutation_colomn PROPERTIES
  FIXTURES_REQUIRED run_apply_permutation_colomn
  LABELS apply_permutation
)

#### Cluster map

##### Without minimum size

add_test(
  NAME run_cluster_map_with_index_map
  COMMAND $<TARGET_FILE:stdp> tool analyze response cluster-map
  ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron_permutated_both.txt ./data/analyze/cluster_map_with_index_map.txt
  --index-file ${CMAKE_SOURCE_DIR}/test/data/analyze/sort_index_neuron_excitatory.txt
  --input-size 100
  --correlation-threshold 0.9
)
set_tests_properties(
  run_cluster_map_with_index_map PROPERTIES
  FIXTURES_SETUP run_cluster_map_with_index_map
  FIXTURES_REQUIRED data_directory
  LABELS cluster_map
)

add_test(
  NAME compare_cluster_map_with_index_map
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/cluster_map_with_index_map.txt ./data/analyze/cluster_map_with_index_map.txt
)
set_tests_properties(
  compare_cluster_map_with_index_map PROPERTIES
  FIXTURES_REQUIRED run_cluster_map_with_index_map
  LABELS cluster_map
)

##### With minimum size

add_test(
  NAME run_cluster_map_with_minimum_size
  COMMAND $<TARGET_FILE:stdp> tool analyze response cluster-map
  ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron_permutated_both.txt ./data/analyze/cluster_map_with_minimum_size.txt
  --index-file ${CMAKE_SOURCE_DIR}/test/data/analyze/sort_index_neuron_excitatory.txt
  --input-size 100
  --correlation-threshold 0.9
  --minimum-cluster-size 5
)
set_tests_properties(
  run_cluster_map_with_minimum_size PROPERTIES
  FIXTURES_SETUP run_cluster_map_with_minimum_size
  FIXTURES_REQUIRED data_directory
  LABELS cluster_map
)

add_test(
  NAME compare_cluster_map_with_minimum_size
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/cluster_map_with_minimum_size.txt ./data/analyze/cluster_map_with_minimum_size.txt
)
set_tests_properties(
  compare_cluster_map_with_minimum_size PROPERTIES
  FIXTURES_REQUIRED run_cluster_map_with_minimum_size
  LABELS cluster_map
)

#### Cut

##### Excitatory only

add_test(
  NAME run_response_cut_excitatory
  COMMAND $<TARGET_FILE:stdp> tool analyze response cut
  ${CMAKE_SOURCE_DIR}/test/data/pre-tested/resps_test.txt
  --excitatory-only-output ./data/analyze/response_excitatory.txt
  --excitatory-neuron-number 100
  --inhibitory-neuron-number 20
  --stimulation-number 1000
)
set_tests_properties(
  run_response_cut_excitatory PROPERTIES
  FIXTURES_SETUP run_response_cut_excitatory
  FIXTURES_REQUIRED data_directory
  LABELS response_cut
)

add_test(
  NAME compare_response_excitatory
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/response_excitatory.txt ./data/analyze/response_excitatory.txt
)
set_tests_properties(
  compare_response_excitatory PROPERTIES
  FIXTURES_REQUIRED run_response_cut_excitatory
  LABELS response_cut
)

##### Inhibitory only

add_test(
  NAME run_response_cut_inhibitory
  COMMAND $<TARGET_FILE:stdp> tool analyze response cut
  ${CMAKE_SOURCE_DIR}/test/data/pre-tested/resps_test.txt
  --inhibitory-only-output ./data/analyze/response_inhibitory.txt
  --excitatory-neuron-number 100
  --inhibitory-neuron-number 20
  --stimulation-number 1000
)
set_tests_properties(
  run_response_cut_inhibitory PROPERTIES
  FIXTURES_SETUP run_response_cut_inhibitory
  FIXTURES_REQUIRED data_directory
  LABELS response_cut
)

add_test(
  NAME compare_response_inhibitory
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/response_inhibitory.txt ./data/analyze/response_inhibitory.txt
)
set_tests_properties(
  compare_response_inhibitory PROPERTIES
  FIXTURES_REQUIRED run_response_cut_inhibitory
  LABELS response_cut
)

#### Correlation matrix

add_test(
  NAME run_correlation_matrix
  COMMAND $<TARGET_FILE:stdp> tool analyze response correlation-matrix
  ${CMAKE_SOURCE_DIR}/test/data/analyze/response_excitatory.txt
  ${CMAKE_SOURCE_DIR}/test/data/analyze/response_excitatory.txt
  --neuron ./data/analyze/correlationMatrix_neuron.txt
  --stimulation ./data/analyze/correlationMatrix_stimulation.txt
  --neuron-number 100
  --stimulation-number 1000
)
set_tests_properties(
  run_correlation_matrix PROPERTIES
  FIXTURES_SETUP run_correlation_matrix
  FIXTURES_REQUIRED data_directory
  LABELS correlation_matrix
)

add_test(
  NAME compare_correlation_matrix_neuron
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_neuron.txt ./data/analyze/correlationMatrix_neuron.txt
)
set_tests_properties(
  compare_correlation_matrix_neuron PROPERTIES
  FIXTURES_REQUIRED run_correlation_matrix
  LABELS correlation_matrix
)

add_test(
  NAME compare_correlation_matrix_stimulation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/correlationMatrix_stimulation.txt ./data/analyze/correlationMatrix_stimulation.txt
)
set_tests_properties(
  compare_correlation_matrix_stimulation PROPERTIES
  FIXTURES_REQUIRED run_correlation_matrix
  LABELS correlation_matrix
)

### Divide line

#### Run divide line

add_test(
  NAME run_divide_line
  COMMAND $<TARGET_FILE:stdp> tool analyze divide-line
  ${CMAKE_SOURCE_DIR}/test/data/analyze/cluster_map_with_index_map.txt ./data/analyze/divide-line
  --number-output ./data/analyze/divide-line-number.txt
  --zero-padding 0
)
set_tests_properties(
  run_divide_line PROPERTIES
  FIXTURES_REQUIRED data_directory
  FIXTURES_SETUP run_divide_line
  LABELS divide_line
)

add_test(
  NAME compare_divide_line_number
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/divide-line-number.txt ./data/analyze/divide-line-number.txt
)
set_tests_properties(
  compare_divide_line_number PROPERTIES
  FIXTURES_REQUIRED run_divide_line
  LABELS divide_line
)

add_test(
  NAME compare_divide_line
  COMMAND diff -r ${CMAKE_SOURCE_DIR}/test/data/analyze/divide-line ./data/analyze/divide-line
)
set_tests_properties(
  compare_divide_line PROPERTIES
  FIXTURES_REQUIRED run_divide_line
  LABELS divide_line
)


#### feedforward

##### export

add_test(
  NAME run_weight_feedforward_export
  COMMAND $<TARGET_FILE:stdp> tool analyze weight feedforward export
  ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.txt
  --on-center-directory ./data/weight/on
  --off-center-directory ./data/weight/off
  --diff-directory ./data/weight/diff
  --edge-length 17
  --excitatory-neuron-number 100
  --inhibitory-neuron-number 20
  --zero-padding 0
)
set_tests_properties(
  run_weight_feedforward_export PROPERTIES
  FIXTURES_REQUIRED data_directory
  FIXTURES_SETUP run_weight_feedforward_export
  LABELS weight_feedforward_export
)

add_test(
  NAME compare_weight_feedforward_export_on
  COMMAND diff -r ${CMAKE_SOURCE_DIR}/test/data/weight/on ./data/weight/on
)
set_tests_properties(
  compare_weight_feedforward_export_on PROPERTIES
  FIXTURES_REQUIRED run_weight_feedforward_export
  LABELS weight_feedforward_export
)

add_test(
  NAME compare_weight_feedforward_export_off
  COMMAND diff -r ${CMAKE_SOURCE_DIR}/test/data/weight/off ./data/weight/off
)
set_tests_properties(
  compare_weight_feedforward_export_off PROPERTIES
  FIXTURES_REQUIRED run_weight_feedforward_export
  LABELS weight_feedforward_export
)

add_test(
  NAME compare_weight_feedforward_export_diff
  COMMAND diff -r ${CMAKE_SOURCE_DIR}/test/data/weight/diff ./data/weight/diff
)
set_tests_properties(
  compare_weight_feedforward_export_diff PROPERTIES
  FIXTURES_REQUIRED run_weight_feedforward_export
  LABELS weight_feedforward_export
)

#### transpose
add_test(
  NAME run_transpose
  COMMAND $<TARGET_FILE:stdp> tool analyze transpose
  ${CMAKE_SOURCE_DIR}/test/data/analyze/transpose/before.txt
  ./data/analyze/transpose/after.txt
  --colomn 3
  --row 2
)
set_tests_properties(
  run_transpose PROPERTIES
  FIXTURES_REQUIRED data_directory
  FIXTURES_SETUP run_transpose
  LABELS transpose
)

add_test(
  NAME compare_transpose
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/transpose/after.txt ./data/analyze/transpose/after.txt
)
set_tests_properties(
  compare_transpose PROPERTIES
  FIXTURES_REQUIRED run_transpose
  LABELS transpose
)


### Explore maximum

#### a=b=0.1
add_test(
  NAME run-explore-maximum-0.1-0.1
  COMMAND $<TARGET_FILE:stdp> tool analyze explore-maximum
  --seed 0
  --evaluation-function-parameter-a 0.1
  --evaluation-function-parameter-b 0.1
  --evaluation-function-parameter-sparseness-intensity 0.05
  --evaluation-function-parameter-sparseness-width 10
  --evaluation-function-parameter-smoothness-intensity 1e-6
  --evaluation-function-parameter-standard-derivation-intensity 0
  --gradient-descent
  --delta 10
  --template-response ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/response-0001.txt
  --neuron-number 120
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --initial-input-number 100
  --iteration-number 1
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --presentation-time 200
  --output-file ./data/analyze/explore-maximum/0.1,0.1/last-image.txt
  --save-log-interval 1
  --save-log-directory ./data/analyze/explore-maximum/0.1,0.1/log
  --save-evaluation-file ./data/analyze/explore-maximum/0.1,0.1/evaluation.txt
  --save-correlation-file ./data/analyze/explore-maximum/0.1,0.1/correlation.txt
  --save-sparseness-file ./data/analyze/explore-maximum/0.1,0.1/sparseness.txt
  --save-smoothness-file ./data/analyze/explore-maximum/0.1,0.1/smoothness.txt
  --save-standard-derivation-file ./data/analyze/explore-maximum/0.1,0.1/standard-derivation.txt
  --save-active-neuron-activity-file ./data/analyze/explore-maximum/0.1,0.1/activity-active.txt
  --save-inactive-neuron-activity-file ./data/analyze/explore-maximum/0.1,0.1/activity-inactive.txt
  --save-evaluation-pixel-file ./data/analyze/explore-maximum/0.1,0.1/evaluation-pixel.txt
  --save-response-file ./data/analyze/explore-maximum/0.1,0.1/response.txt
  --delays-file ${CMAKE_SOURCE_DIR}/test/data/learn/delays.txt
)
set_tests_properties(
  run-explore-maximum-0.1-0.1 PROPERTIES
  FIXTURES_REQUIRED data_directory
  FIXTURES_SETUP run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_evaluation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/evaluation.txt ./data/analyze/explore-maximum/0.1,0.1/evaluation.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_evaluation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_correlation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/correlation.txt ./data/analyze/explore-maximum/0.1,0.1/correlation.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_correlation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_sparseness
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/sparseness.txt ./data/analyze/explore-maximum/0.1,0.1/sparseness.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_sparseness PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_smoothness
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/smoothness.txt ./data/analyze/explore-maximum/0.1,0.1/smoothness.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_smoothness PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_standard-derivation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/standard-derivation.txt ./data/analyze/explore-maximum/0.1,0.1/standard-derivation.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_standard-derivation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_evaluation-pixel
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/evaluation-pixel.txt ./data/analyze/explore-maximum/0.1,0.1/evaluation-pixel.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_evaluation-pixel PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_response
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/response.txt ./data/analyze/explore-maximum/0.1,0.1/response.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_response PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_activity-active
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/activity-active.txt ./data/analyze/explore-maximum/0.1,0.1/activity-active.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_activity-active PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_activity-inactive
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/activity-inactive.txt ./data/analyze/explore-maximum/0.1,0.1/activity-inactive.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_activity-inactive PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_last-image
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/last-image.txt ./data/analyze/explore-maximum/0.1,0.1/last-image.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_last-image PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.1_log
  COMMAND diff -r ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.1/log ./data/analyze/explore-maximum/0.1,0.1/log
)
set_tests_properties(
  compare-explore-maximum-0.1-0.1_log PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.1
  LABELS explore-maximum
)

#### a=0.1, b=0.2
add_test(
  NAME run-explore-maximum-0.1-0.2
  COMMAND $<TARGET_FILE:stdp> tool analyze explore-maximum
  --seed 0
  --evaluation-function-parameter-a 0.1
  --evaluation-function-parameter-b 0.2
  --evaluation-function-parameter-sparseness-intensity 0.05
  --evaluation-function-parameter-sparseness-width 10
  --evaluation-function-parameter-smoothness-intensity 1e-6
  --evaluation-function-parameter-standard-derivation-intensity 0
  --gradient-descent
  --delta 10
  --template-response ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/response-0001.txt
  --neuron-number 120
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --initial-input-number 100
  --iteration-number 1
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --presentation-time 200
  --output-file ./data/analyze/explore-maximum/0.1,0.2/last-image.txt
  --save-log-interval 1
  --save-log-directory ./data/analyze/explore-maximum/0.1,0.2/log
  --save-evaluation-file ./data/analyze/explore-maximum/0.1,0.2/evaluation.txt
  --save-correlation-file ./data/analyze/explore-maximum/0.1,0.2/correlation.txt
  --save-sparseness-file ./data/analyze/explore-maximum/0.1,0.2/sparseness.txt
  --save-smoothness-file ./data/analyze/explore-maximum/0.1,0.2/smoothness.txt
  --save-standard-derivation-file ./data/analyze/explore-maximum/0.1,0.2/standard-derivation.txt
  --save-active-neuron-activity-file ./data/analyze/explore-maximum/0.1,0.2/activity-active.txt
  --save-inactive-neuron-activity-file ./data/analyze/explore-maximum/0.1,0.2/activity-inactive.txt
  --save-evaluation-pixel-file ./data/analyze/explore-maximum/0.1,0.2/evaluation-pixel.txt
  --save-response-file ./data/analyze/explore-maximum/0.1,0.2/response.txt
  --delays-file ${CMAKE_SOURCE_DIR}/test/data/learn/delays.txt
)
set_tests_properties(
  run-explore-maximum-0.1-0.2 PROPERTIES
  FIXTURES_REQUIRED data_directory
  FIXTURES_SETUP run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_evaluation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/evaluation.txt ./data/analyze/explore-maximum/0.1,0.2/evaluation.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_evaluation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_correlation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/correlation.txt ./data/analyze/explore-maximum/0.1,0.2/correlation.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_correlation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_sparseness
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/sparseness.txt ./data/analyze/explore-maximum/0.1,0.2/sparseness.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_sparseness PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_smoothness
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/smoothness.txt ./data/analyze/explore-maximum/0.1,0.2/smoothness.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_smoothness PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_standard-derivation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/standard-derivation.txt ./data/analyze/explore-maximum/0.1,0.2/standard-derivation.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_standard-derivation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_evaluation-pixel
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/evaluation-pixel.txt ./data/analyze/explore-maximum/0.1,0.2/evaluation-pixel.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_evaluation-pixel PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_response
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/response.txt ./data/analyze/explore-maximum/0.1,0.2/response.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_response PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_activity-active
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/activity-active.txt ./data/analyze/explore-maximum/0.1,0.2/activity-active.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_activity-active PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_activity-inactive
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/activity-inactive.txt ./data/analyze/explore-maximum/0.1,0.2/activity-inactive.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_activity-inactive PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_last-image
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/last-image.txt ./data/analyze/explore-maximum/0.1,0.2/last-image.txt
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_last-image PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-0.1-0.2_log
  COMMAND diff -r ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/0.1,0.2/log ./data/analyze/explore-maximum/0.1,0.2/log
)
set_tests_properties(
  compare-explore-maximum-0.1-0.2_log PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-0.1-0.2
  LABELS explore-maximum
)

#### With standard derivation
add_test(
  NAME run-explore-maximum-with-sd
  COMMAND $<TARGET_FILE:stdp> tool analyze explore-maximum
  --seed 0
  --evaluation-function-parameter-a 0.1
  --evaluation-function-parameter-b 0.2
  --evaluation-function-parameter-sparseness-intensity 0.05
  --evaluation-function-parameter-sparseness-width 10
  --evaluation-function-parameter-smoothness-intensity 1e-6
  --evaluation-function-parameter-standard-derivation-intensity 0.02
  --gradient-descent
  --delta 10
  --template-response ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/response-0001.txt
  --neuron-number 120
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --initial-input-number 100
  --iteration-number 1
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --presentation-time 200
  --output-file ./data/analyze/explore-maximum/with-sd/last-image.txt
  --save-log-interval 1
  --save-log-directory ./data/analyze/explore-maximum/with-sd/log
  --save-evaluation-file ./data/analyze/explore-maximum/with-sd/evaluation.txt
  --save-correlation-file ./data/analyze/explore-maximum/with-sd/correlation.txt
  --save-sparseness-file ./data/analyze/explore-maximum/with-sd/sparseness.txt
  --save-smoothness-file ./data/analyze/explore-maximum/with-sd/smoothness.txt
  --save-standard-derivation-file ./data/analyze/explore-maximum/with-sd/standard-derivation.txt
  --save-active-neuron-activity-file ./data/analyze/explore-maximum/with-sd/activity-active.txt
  --save-inactive-neuron-activity-file ./data/analyze/explore-maximum/with-sd/activity-inactive.txt
  --save-evaluation-pixel-file ./data/analyze/explore-maximum/with-sd/evaluation-pixel.txt
  --save-response-file ./data/analyze/explore-maximum/with-sd/response.txt
  --delays-file ${CMAKE_SOURCE_DIR}/test/data/learn/delays.txt
)
set_tests_properties(
  run-explore-maximum-with-sd PROPERTIES
  FIXTURES_REQUIRED data_directory
  FIXTURES_SETUP run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_evaluation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/evaluation.txt ./data/analyze/explore-maximum/with-sd/evaluation.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_evaluation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_correlation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/correlation.txt ./data/analyze/explore-maximum/with-sd/correlation.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_correlation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_sparseness
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/sparseness.txt ./data/analyze/explore-maximum/with-sd/sparseness.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_sparseness PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_smoothness
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/smoothness.txt ./data/analyze/explore-maximum/with-sd/smoothness.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_smoothness PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_standard-derivation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/standard-derivation.txt ./data/analyze/explore-maximum/with-sd/standard-derivation.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_standard-derivation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_evaluation-pixel
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/evaluation-pixel.txt ./data/analyze/explore-maximum/with-sd/evaluation-pixel.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_evaluation-pixel PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_response
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/response.txt ./data/analyze/explore-maximum/with-sd/response.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_response PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_activity-active
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/activity-active.txt ./data/analyze/explore-maximum/with-sd/activity-active.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_activity-active PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_activity-inactive
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/activity-inactive.txt ./data/analyze/explore-maximum/with-sd/activity-inactive.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_activity-inactive PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_last-image
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/last-image.txt ./data/analyze/explore-maximum/with-sd/last-image.txt
)
set_tests_properties(
  compare-explore-maximum-with-sd_last-image PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-sd_log
  COMMAND diff -r ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-sd/log ./data/analyze/explore-maximum/with-sd/log
)
set_tests_properties(
  compare-explore-maximum-with-sd_log PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-sd
  LABELS explore-maximum
)

#### With noise
add_test(
  NAME run-explore-maximum-with-noise
  COMMAND $<TARGET_FILE:stdp> tool analyze explore-maximum
  --seed 0
  --evaluation-function-parameter-a 0.1
  --evaluation-function-parameter-b 0.1
  --evaluation-function-parameter-sparseness-intensity 0.05
  --evaluation-function-parameter-sparseness-width 10
  --evaluation-function-parameter-smoothness-intensity 1e-6
  --evaluation-function-parameter-standard-derivation-intensity 0
  --search-with-noise
  --number-of-noise 100
  --stddev 10
  --template-response ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/response-0001.txt
  --neuron-number 120
  --input-file ${PROJECT_SOURCE_DIR}/patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat
  --initial-input-number 100
  --iteration-number 1
  --lateral-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/w.dat
  --feedforward-weight ${CMAKE_SOURCE_DIR}/test/data/pre-learned/wff.dat
  --presentation-time 200
  --output-file ./data/analyze/explore-maximum/with-noise/last-image.txt
  --save-log-interval 1
  --save-log-directory ./data/analyze/explore-maximum/with-noise/log
  --save-evaluation-file ./data/analyze/explore-maximum/with-noise/evaluation.txt
  --save-correlation-file ./data/analyze/explore-maximum/with-noise/correlation.txt
  --save-sparseness-file ./data/analyze/explore-maximum/with-noise/sparseness.txt
  --save-smoothness-file ./data/analyze/explore-maximum/with-noise/smoothness.txt
  --save-standard-derivation-file ./data/analyze/explore-maximum/with-noise/standard-derivation.txt
  --save-active-neuron-activity-file ./data/analyze/explore-maximum/with-noise/activity-active.txt
  --save-inactive-neuron-activity-file ./data/analyze/explore-maximum/with-noise/activity-inactive.txt
  --save-evaluation-pixel-file ./data/analyze/explore-maximum/with-noise/evaluation-pixel.txt
  --save-response-file ./data/analyze/explore-maximum/with-noise/response.txt
  --delays-file ${CMAKE_SOURCE_DIR}/test/data/learn/delays.txt
)
set_tests_properties(
  run-explore-maximum-with-noise PROPERTIES
  FIXTURES_REQUIRED data_directory
  FIXTURES_SETUP run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_evaluation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/evaluation.txt ./data/analyze/explore-maximum/with-noise/evaluation.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_evaluation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_correlation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/correlation.txt ./data/analyze/explore-maximum/with-noise/correlation.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_correlation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_sparseness
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/sparseness.txt ./data/analyze/explore-maximum/with-noise/sparseness.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_sparseness PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_smoothness
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/smoothness.txt ./data/analyze/explore-maximum/with-noise/smoothness.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_smoothness PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_standard-derivation
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/standard-derivation.txt ./data/analyze/explore-maximum/with-noise/standard-derivation.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_standard-derivation PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_evaluation-pixel
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/evaluation-pixel.txt ./data/analyze/explore-maximum/with-noise/evaluation-pixel.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_evaluation-pixel PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_response
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/response.txt ./data/analyze/explore-maximum/with-noise/response.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_response PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_activity-active
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/activity-active.txt ./data/analyze/explore-maximum/with-noise/activity-active.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_activity-active PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_activity-inactive
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/activity-inactive.txt ./data/analyze/explore-maximum/with-noise/activity-inactive.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_activity-inactive PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_last-image
  COMMAND diff ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/last-image.txt ./data/analyze/explore-maximum/with-noise/last-image.txt
)
set_tests_properties(
  compare-explore-maximum-with-noise_last-image PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)

add_test(
  NAME compare-explore-maximum-with-noise_log
  COMMAND diff -r ${CMAKE_SOURCE_DIR}/test/data/analyze/explore-maximum/with-noise/log ./data/analyze/explore-maximum/with-noise/log
)
set_tests_properties(
  compare-explore-maximum-with-noise_log PROPERTIES
  FIXTURES_REQUIRED run-explore-maximum-with-noise
  LABELS explore-maximum
)
