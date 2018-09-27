#=============================================================================
# run_pipeline.tcl 
#=============================================================================
# @brief: A Tcl script for synthesizing the pipelined digit recongnition design.

# Project name
set hls_prj digitrec_pipeline.prj

# Open/reset the project
open_project ${hls_prj} -reset

# Top function of the design is "dut"
set_top dut

# Add design and testbench files
add_files digitrec.cpp
add_files -tb digitrec_test.cpp
add_files -tb data

open_solution "solution1"
# Use Zynq device
set_part {xc7z020clg484-1}

# Target clock period is 10ns
create_clock -period 10

### You can insert your own directives here ###
set_directive_inline update_knn
set_directive_inline knn_vote

#set_directive_array_partition -type complete -dim 1 digitrec knn_set
set_directive_array_partition -type complete digitrec knn_set
set_directive_array_partition -type complete -dim 1 digitrec training_data
set_directive_unroll digitrec/L10

set_directive_loop_unroll update_knn/UPDATE_DIFF_LOOP
set_directive_loop_unroll update_knn/UPDATE_DIST_LOOP
set_directive_loop_unroll knn_vote/VOTE_INIT_DIST_LOOP
set_directive_loop_unroll knn_vote/VOTE_INIT_VOTE_LOOP
set_directive_loop_unroll knn_vote/VOTE_MIN_DIST_DIGIT_LOOP
set_directive_loop_unroll knn_vote/VOTE_MIN_DIST_CONST_LOOP
set_directive_loop_unroll knn_vote/VOTE_MIN_DIST_CUR_CONST_LOOP
set_directive_loop_unroll knn_vote/VOTE_CALC_VOTE_LOOP
set_directive_loop_unroll knn_vote/VOTE_FIND_MAX_VOTE_LOOP

# Pipeline the main loop!
set_directive_pipeline digitrec/L1800


############################################

# Simulate the C++ design
#csim_design
# Synthesize the design
csynth_design
# Co-simulate the design
#cosim_design
exit
