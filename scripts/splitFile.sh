#!/bin/bash

num_of_arg=$#

if [ ${num_of_arg} -eq 3 ]
then
  echo "Arguments given are:- ."
  echo "1. Input file - ${1}"
  echo "2. Output folder - ${2}"
  echo "3. Number of output files - ${3}"
else
  echo "Three arguments are expected."
  echo "1. Input file."
  echo "2. Output folder."
  echo "3. Number of output files" 
  exit 1
fi

total_lines=$(cat ${1} | wc -l)
lines_each_op_file=$(expr ${total_lines} / ${3})

split -l "${lines_each_op_file}" "${2}"