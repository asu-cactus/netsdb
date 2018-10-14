#!/bin/bash

pem_file=$1

user=ubuntu
ip_len_valid=3

arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<=$length ; i++ ))
do
        ip_addr=${arr[i]}
        if [ ${#ip_addr} -gt "$ip_len_valid" ]
        then
                echo -e "\n+++++++++++ reboot server: $ip_addr"
                ssh -i $pem_file $user@$ip_addr 'sudo reboot'
        fi
done

echo -e "\nWaiting 60 seconds..."
sleep 60

