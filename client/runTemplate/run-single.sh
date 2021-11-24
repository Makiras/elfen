#!/bin/bash

ITERs=5
QPS=100
# Tasks="wiki.1M.nostopwords.term.tasks"
# Tasks="wikimedium.10M.nostopwords.tasks"
Tasks="wiki.s.10M.nostopwords.tasks"

# Perf colocation 
for ((QPS=420;QPS>=10;QPS=QPS-15)); do
    # lucene will running on CPU 0
    taskset -c 16 python2 -u ../script/sendTasks.py "../script/${Tasks}" 10.16.0.184 7777 ${QPS} 1000000 200000 test_${QPS}_${ITERs} ${ITERs} order > py_log_${QPS}_${ITERs}.txt &
    pyPid=$!
    # sudo cgexec -g cpu:/kubepods.slice/kubepodsburstable.slice/matrix time taskset -c 32 ../../microbench/matrix_nonap > c_log_${QPS}_${ITERs}.txt  2>&1 &
    # matrix_pid=$!
    # matrix_pgid=$(ps -o pgid= $matrix_pid | xargs)
    # sleep 5  # sleep for perf call matrix_nonap finished
    # matrix_spid=$(sudo pstree -p ${matrix_pid} | grep -E -o "[0-9]+" | tail -1 )
    # echo "matrix_nonap running on CPU 32 with sudo pid $matrix_pid, gid $matrix_pgid, real_pid $matrix_spid"
    wait ${pyPid}
    # sudo kill -INT ${matrix_spid}

    # Move the result to the result folder
    mkdir -p ${QPS}_${ITERs}_single
    # mv c_log_${QPS}_${ITERs}.txt  ${QPS}_${ITERs}_single/
    mv py_log_${QPS}_${ITERs}.txt ${QPS}_${ITERs}_single/
    mv test_${QPS}_${ITERs} ${QPS}_${ITERs}_single/

    # Parse Task
    cd ${QPS}_${ITERs}_single
    python2 ../../script/parseTasks.py test_${QPS}_${ITERs}
    cd ../
done