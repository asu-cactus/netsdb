#!/bin/bash
# by Jia


user=ubuntu

arr=($(awk '{print $0}' $HADOOP_HOME/etc/hadoop/slaves))
length=${#arr[@]}
echo "There are $length servers"
for (( i=0 ; i<$length ; i++ ))
do
        host=${arr[i]}
        echo -e "\n+++++++++++ reconfig: $host"
#        ssh $host 'java -version'
#        ssh ubuntu@$host 'mkdir /home/ubuntu/hadoop-data'     
#         scp etc/hadoop/slaves ubuntu@$host:$HADOOP_HOME/etc/hadoop/
#          scp etc/hadoop/yarn-env.sh ubuntu@$host:$HADOOP_HOME/etc/hadoop/
#           scp etc/hadoop/core-site.xml ubuntu@$host:$HADOOP_HOME/etc/hadoop/

#         scp etc/hadoop/hdfs-site.xml ubuntu@$host:$HADOOP_HOME/etc/hadoop/
#           scp etc/hadoop/mapred-site.xml ubuntu@$host:$HADOOP_HOME/etc/hadoop/
         scp etc/hadoop/yarn-site.xml ubuntu@$host:$HADOOP_HOME/etc/hadoop/
#        ssh $host '/home/ubuntu/hadoop-2.7.3/sbin/hadoop-daemons.sh --config /home/ubuntu/hadoop-2.7.3/etc/hadoop --script hdfs stop datanode'
done

