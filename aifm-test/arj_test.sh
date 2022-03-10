#!/bin/bash

source shared.sh


function run_single_test {
    echo "Running test $1..."
    rerun_local_iokerneld
    if [[ $1 == *"tcp"* ]]; then
        rerun_mem_server
    fi
    run_program ./bin/$1 >> test_run.out
    
}

function cleanup {
    kill_local_iokerneld
    kill_mem_server
}

test=test_array_add

run_single_test $test
cleanup

exit 0
