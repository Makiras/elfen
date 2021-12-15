#!/bin/bash

# iterate all strategies
for c in $(ls -d */); do
    cd $c
    echo "Concluding strategy $c"

    echo "QPS,RealQPS,p50,p95,p99,l50,l95,l99,utilization,IPC,send time,recv time,matrix,user_t,sys_t,b_utilization" > conclusion.csv
    
    # iterate all qps conditions, sort by qps (math)
    for t in $(ls -dv */); do
        cd $t
        echo " qps $t"
        # read the number of qps // qps, realqps, p50,p95,p99, l50,l95,l99, c50,c95,c99, utiliztion,IPC
        qps_state=($(cat qps-latency.csv | sed -n '2p' |grep -o -E '[+-]?([0-9]*)\.?[0-9]+'))
        # echo ${qps_state[*]}

        # read the number send/recv time and times // t_send, n_send, n_recv, t_recv
        py_state=($(cat py_log_*|egrep "Done|send" | grep -o -E '[+-]?([0-9]*)\.?[0-9]+'))
        # echo py_state
        if [ ${py_state[1]} -ne ${py_state[2]} ]; then
            echo "Error: send/recv not equal"
            exit 1
        fi
        py_state=(${py_state[0]} ${py_state[3]})
        

        # read the state of batch workload // matrix_times, user_time, sys_time, total_time, utilization, avgt,avgd,maxres
        f_c_log=$(ls c_log* 2>/dev/null)
        if [ ! -f "$f_c_log" ]; then
            # echo "No c_log file"
            batch_state=()
        else
            batch_state=($(cat $f_c_log | egrep "calc matrix|user" |grep -o -E '[+-]?(([0-9]*)\.?[0-9]:?)*'))
            echo ${batch_state[*]}
        fi
        cd ../

        # replace space split with comma
        result=(${qps_state[*]:0:8} ${qps_state[*]:11:2} ${py_state[*]})
        
        if [ ${#batch_state[*]} -ne 0 ]; then
	    utilization=$(echo "${batch_state[4]}" | awk '{print 0.01*$1}')
            result+=(${batch_state[*]:0:3} ${utilization})
        fi
        result=${result[*]}
        echo ${result// /,}  >> conclusion.csv

    done
    cd ../
done
