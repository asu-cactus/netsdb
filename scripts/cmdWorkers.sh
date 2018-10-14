#/bin/bash
# by Jia


user=ubuntu
pemFile=$1
command=$2
arr=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<$length ; i++ ))
do
        host=${arr[i]}
        echo -e "\n+++++++++++ run command:$command at $host"
        ssh -i $pemFile $host $command
done
