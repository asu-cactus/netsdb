#!/bin/bash

num_of_arg=$#

if [ ${num_of_arg} -eq 2 ]; then
  echo "Task to execute is ${1}"
  echo "Install Directory is ${2}"
else
  echo "Two arguments are expected"
  echo "1. Task to execute - 'flush' or 'load'"
  echo "2. PDB Install Directory"
  exit 1
fi

self_ip=$(ifconfig | sed -En 's/127.0.0.1//;s/.*inet (addr:)?(([0-9]*\.){3}[0-9]*).*/\2/p')
echo "PDB install directory is ${PDB_INSTALL}"

if [ "${1}" == "flush" ]; then
  aws s3 cp --region us-east-2 --recursive \
  ${2}/pdbRoot_localhost_8108/  \
  s3://lachesisstore/worker_data/"${self_ip}"/pdbRoot_localhost_8108/
elif [ "${1}" == "load" ]; then
  aws s3 cp --region us-east-2 --recursive \
  s3://lachesisstore/worker_data/"${self_ip}"/pdbRoot_localhost_8108/ \
  ${2}/pdbRoot_localhost_8108/
fi

