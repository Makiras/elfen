#!/bin/bash

# iterate all strategies
for c in $(ls -d */); do
    cd $c
    echo "Reparsing strategy $c"
    
    # iterate all qps conditions
    for t in $(ls -d */); do
        cd $t
        f=$(ls test*)
        echo "  condition $t, parsing $f"
        python2 ../../../client/script/parseTasks.py $f &
        cd ../
    done
    wait
    cd ../
done