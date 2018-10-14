#!/bin/bash
pemFile=$1
PDB_SLEEP_TIME=40
pkill -9  test404
pkill -9  test603
pkill -9  pdb-cluster
pkill -9  pdb-server

$PDB_HOME/bin/pdb-cluster localhost 8108 N $pemFile 1.5 &

echo "#####################################"
echo "To sleep for 100 seconds in total for all ssh to return"
echo "#####################################"

sleep $PDB_SLEEP_TIME

for x in {1..10};
do
   if pgrep -x "pdb-cluster" > /dev/null
   then
       echo "master is started!"
       exit 0
   fi
   sleep 10
done

echo "master hasn't started! It could be that ssh takes too long time. Please retry!"


