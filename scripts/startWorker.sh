#!/bin/bash
nodeId=$1
numThreads=$2
sharedMemSize=$3
master_ip=$4
ip_addr=$5

echo -e "+++++++++++ start server: $ip_addr"
echo -e "+++++++++++ to kill existing processes"
pkill -9 test603
pkill -9 test404
pkill -9 pdb-server
pkill -9 pdb-cluster 

echo -e "+++++++++++ to start a pdb worker"
echo "bin/pdb-server $nodeId $numThreads $sharedMemSize $master_ip $ip_addr &"
if [ -n "${PDB_SSH_FOREGROUND}" ]; then
   bin/pdb-server $nodeId $numThreads $sharedMemSize $master_ip $ip_addr
else
   nohup bin/pdb-server $nodeId $numThreads $sharedMemSize $master_ip $ip_addr  >> log.out 2>&1 < /dev/null &
fi
