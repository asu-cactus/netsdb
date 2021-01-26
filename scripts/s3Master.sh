#!/bin/bash

num_of_arg=$#

if [ ${num_of_arg} -eq 3 ]; then
  echo "Private key given is ${1}"
  echo "Task to execute is ${2}"
  echo "AWS S3 bucket name is ${3}"
else
  echo "Two argument is expected"
  echo "1. Private key to SSH into the worker machines."
  echo "2. Task to execute - 'flush' or 'load'"
  echo "3. AWS S3 bucket name"
  exit 1
fi

workers=($(awk '{print $0}' $PDB_HOME/conf/serverlist))
echo "There are ${#workers[@]} worker nodes"

for w in ${workers[@]}; do
  ssh -i ${1} ${w} "bash $PDB_INSTALL/scripts/s3Worker.sh ${2} ${PDB_INSTALL} ${3}" &
  echo "Process started for ${w} worker with process id ${!}"
done

wait
echo "Process on all the workers is completed."

