#!/bin/bash

pem_file=$1
masterIp=$2
numThreads=$3
sharedMem=$4
user=`id -u -n`
ip_len_valid=3
pdb_dir=$PDB_INSTALL
PDB_SSH_SLEEP=10

# By default disable strict host key checking
if [ "$PDB_SSH_OPTS" = "" ]; then
  PDB_SSH_OPTS="-o StrictHostKeyChecking=no"
fi

if [ -z ${pem_file} ];
    then echo "ERROR: please provide at least two parameters: one is your the path to your pem file and the other is the master IP";
    echo "Usage: scripts/startWorkers.sh #pem_file #masterIp #threadNum #sharedMemSize";
    exit -1;
fi

if [ -z ${masterIp} ];
    then echo "ERROR: please provide at least two parameters: one is the path to your pem file and the other is the master IP";
    echo "Usage: scripts/startWorkers.sh #pem_file #masterIp #threadNum #sharedMemSize";
    exit -1;
fi
    
if [ -z ${numThreads} ];
     then numThreads=4;
fi

if [ -z ${sharedMem} ];
     then sharedMem=4096;
fi

arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<=$length ; i++ ))
do
        
        ip_addr=${arr[i]}
        if [ ${#ip_addr} -gt "$ip_len_valid" ]
        then
                echo -e "+++++++++++ to install or update libraries: $ip_addr"
                ssh -i $pem_file $user@$ip_addr "sudo apt-get update"
                ssh -i $pem_file $user@$ip_addr "sudo apt-get install libssl-dev libsnappy1v5 libsnappy-dev libeigen3-dev libgsl-dev"
                echo -e "\n+++++++++++ start server: $ip_addr"
                ssh -i $pem_file $PDB_SSH_OPTS $user@$ip_addr "cd $pdb_dir;  scripts/startWorker.sh $i  $numThreads $sharedMem $masterIp $ip_addr &" &
                sleep $PDB_SSH_SLEEP
                ssh -i $pem_file $user@$ip_addr $pdb_dir/scripts/checkProcess.sh pdb-server
                
        fi
done

echo "servers are started"
