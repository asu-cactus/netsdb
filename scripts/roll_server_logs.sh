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


if [ "$2" = "n" ]
then
    cp cluster.out cluster-nopart.out
    > cluster.out
elif [ "$2" = "p" ]
then
    cp cluster.out cluster-part.out
    > cluster.out
elif [ "$2" = "c" ]
then
    rm -rf *.out
else
    > client.out
    > client-part.out
    > cluster.out
fi


arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<=$length ; i++ ))
do
        ip_addr=${arr[i]}
        if [ ${#ip_addr} -gt "$ip_len_valid" ]
        then
            if [ "$2" = "n" ]
            then
                echo -e "\n+++++++++++ rolling server non-partitioned logs: $ip_addr"
                ssh $PDB_SSH_OPTS $user@$ip_addr "cd $pdb_dir; cp log.out log-$ip_addr-no-partition.out; > log.out"
            elif [ "$2" = "p" ]
            then
                echo -e "\n+++++++++++ rolling server partitioned logs: $ip_addr"
                ssh $PDB_SSH_OPTS $user@$ip_addr "cd $pdb_dir; cp log.out log-$ip_addr-partition.out; > log.out"
            elif [ "$2" = "c" ]
            then
                echo -e "\n+++++++++++ clearing server logs: $ip_addr"
                ssh $PDB_SSH_OPTS $user@$ip_addr "cd $pdb_dir; rm -rf *.out"
            else
                echo -e "\n+++++++++++ rolling server logs: $ip_addr"
                ssh $PDB_SSH_OPTS $user@$ip_addr "cd $pdb_dir; > log.out"
            fi
        fi
done


