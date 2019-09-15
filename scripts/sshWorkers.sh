#!/bin/bash
# by Jia


user=`id -u -n`

arr=($(awk '{print $0}' $HADOOP_HOME/etc/hadoop/slaves))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<$length ; i++ ))
do
        host=${arr[i]}
        echo -e "\n+++++++++++ reconfig: $host"
         scp etc/hadoop/yarn-site.xml $user@$host:$HADOOP_HOME/etc/hadoop/
done

