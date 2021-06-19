#!/bin/bash

pem_file=$1
pdb_dir=$PDB_INSTALL
user=`id -u -n`
ip_len_valid=3



# By default disable strict host key checking
if [ "$PDB_SSH_OPTS" = "" ]; then
  PDB_SSH_OPTS="-o StrictHostKeyChecking=no"
fi

if [ -z ${pem_file} ];
then
  PDB_SSH_OPTS=$PDB_SSH_OPTS
else
  PDB_SSH_OPTS="-i ${pem_file} $PDB_SSH_OPTS"
fi


arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<=$length ; i++ ))
do
        ip_addr=${arr[i]}
        if [ ${#ip_addr} -gt "$ip_len_valid" ]
        then
            echo -e "\n+++++++++++ rolling server logs: $ip_addr"
            ssh $PDB_SSH_OPTS $user@$ip_addr "cd $pdb_dir; cp log.out ~/log-${ip_addr}-${2}mem-${3}thds-${4}models-${5}rn-${6}.out; > log.out; tree -sa pdbRoot_localhost_8108 > ~/dir-${ip_addr}-${2}mem-${3}thds-${4}models-${5}rn-${6}.out"
        fi
done


