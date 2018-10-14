#/bin/bash
# by Jia


user=ubuntu
pemFile=$1

arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<$length ; i++ ))
do
        host=${arr[i]}
        echo -e "\n+++++++++++ reconfig: $host"
        
        ssh -i $pemFile $host 'sudo swapoff -a'
        #ssh $host 'sudo dd if=/dev/zero of=/swapfile bs=1M count=8192'
        ssh -i $pemFile $host 'sudo mkswap /swapfile'
        ssh -i $pemFile $host 'sudo swapon /swapfile'
done
