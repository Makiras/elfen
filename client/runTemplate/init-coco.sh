#!/bin/bash

# Create Lucene cgroup
mkdir -p /sys/fs/cgroup/cpu/kubepods.slice/kubepodsburstable.slice/lucene
echo 2 > /sys/fs/cgroup/cpu/kubepods.slice/kubepodsburstable.slice/lucene/cpu.coflag # set coflag to 2, priority to Online

# Create Matrix cgroup
mkdir -p /sys/fs/cgroup/cpu/kubepods.slice/kubepodsburstable.slice/matrix # no need to set coflag, priority is normal by default

# Enable colocation scheduling
echo 1 > /proc/sys/kernel/sched_colocation_enabled

# Start Lucene
cd ../../lucene/util/
cgexec -g cpu:/kubepods.slice/kubepodsburstable.slice/lucene ./scripts/runSearchServer.sh