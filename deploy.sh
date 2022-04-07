#!/bin/bash

# Extract the Input File
#tar -xf ../inputs/input_native.tar

# Compile the Project
make -f Makefile

# --------- AIFM ----------
# Prepare
all_passed=1
source ~/AIFM/aifm/shared.sh

rerun_local_iokerneld
rerun_mem_server

# Run Fluidanimate
run_program ./clomp 1 -1 1 400 32 1 100
#run_program gdb ./clomp 

sleep 3

# Clean Up
kill_local_iokerneld
kill_mem_server

if [[ $all_passed -eq 1 ]]; then
    exit 0
else
    exit -1
fi
