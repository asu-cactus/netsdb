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
        echo -e "\n+++++++++++ ls: $host"
        sleep 1 
        ssh -i $pemFile $host 'ps aux | grep pdb-server'
        sleep 1
        ssh -i $pemFile $host "cat $PDB_INSTALL/log.out"
       
done
