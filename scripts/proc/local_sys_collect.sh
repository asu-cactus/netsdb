#!/bin/sh
# by Jia


echo "[INFO] local sys collect"
cpu_num=`grep -c 'model name' /proc/cpuinfo`
echo -e "CPUNumber:\t $cpu_num"
grep 'MemTotal' /proc/meminfo
grep 'model name' /proc/cpuinfo
