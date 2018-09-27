#=============================================================================
# run-fixed.tcl 
#=============================================================================
# @brief: A Tcl script for fixed-point experiments.
#
# @desc: This script runs a batch of simulation & synthesis experiments
# to explore trade-offs between accuracy, performance, and area for 
# fixed-point implementation of the CORDIC core.
#
# 1. The user specifies a list of bitwidth pairs, i.e., (TOT_WIDTH, INT_WIDTH)
# 2. Results are collected in a single file ./result/fixed-result.csv
# 3. out.dat from each individual experiment is also copied to ./result

#------------------------------------------------------
# Set result filename and clean old data
#------------------------------------------------------
set filename "opt_result.csv"
file delete -force "./result/${filename}"

#-----------------------------------------------------
# You can specify a set of bitwidth configurations to 
# explore in the batch experiments. 
# Each configuration (line) is defined by a pair in  
# total bitwidth and integer bitwidth
#-----------------------------------------------------

# set total bitwidth and integer bitwidth
set TOT_W 32
set INT_W 8

# Define the bitwidth macros from CFLAGs
set CFLAGS "-DNUM_ITER=20 -DFIXED_TYPE -DTOT_WIDTH=${TOT_W} -DINT_WIDTH=${INT_W}"

# Project name
set hls_prj "opt_fixed_${TOT_W}_${INT_W}_20.prj"

# Open/reset the project
open_project ${hls_prj} -reset
# Top function of the design is "cordic"
set_top cordic

# Add design and testbench files
add_files cordic.cpp -cflags $CFLAGS
add_files -tb cordic_test.cpp -cflags $CFLAGS

open_solution "solution1"
# Use Zynq device
set_part {xc7z020clg484-1}

# Target clock period is 10ns
create_clock -period 10

### INSERT OPTIMIZATION DIRECTIVES HERE ###

# Partition the constant array for better memory access latency. 
# Turns out this does not affect the generated HW. Maybe vivado_hls
# automatically stores the table in individual FFs?

#set_directive_array_partition -type complete cordic cordic_ctab

# Try to pipeline the loop with II = 1. -rewind instructs vivado_hls
# to generate a pipeline that does not wait between the end of the 
# previous transaction and the start of the next transaction.
# This directive will be cancelled if the loop has been fully unrolled

#set_directive_pipeline -II 1 -rewind cordic/FIXED_STEP_LOOP

# Fully unroll the loop. Overlaps with loop-pipelining.

#set_directive_unroll -skip_exit_check -factor 20 cordic/FIXED_STEP_LOOP

# Pipeline the cordic() function

set_directive_pipeline -II 1 cordic

###########################################

# Simulate the C++ design
csim_design
# Synthesize the design
csynth_design

# We will skip C-RTL cosimulation for now
#cosim_design

#---------------------------------------------
# Collect & dump out results from HLS reports
#---------------------------------------------
set argv [list $filename $hls_prj]
set argc 2
source "./script/collect_result.tcl"

quit
