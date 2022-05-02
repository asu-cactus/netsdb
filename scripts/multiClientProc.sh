#!/bin/bash

num_of_arg=$#

if [ ${num_of_arg} -ge 6 ]
then
  echo "Arguments given are:- ."
  echo "1. Program to execute - ${1}"
  echo "2. managerIP - ${2}"
  echo "3. managerPort - ${3}"
  echo "4. inputFolder - ${4}"
  echo "5. whetherToPrepartitionData - ${5}"
  echo "6. whetherToRegisterLibraries - ${6}"
else
  echo "Six arguments are expected."
  echo "1. Program to execute - path to the binary file"
  echo "2. managerIP - IP of the manager"
  echo "3. managerPort - Port of the manager"
  echo "4. inputFolder - Folder containing the part files of input set"
  echo "5. whetherToPrepartitionData - Y yes, N no"
  echo "6. whetherToRegisterLibraries - Y yes, N no"
  exit 1
fi

# Read the list of part files within the input directory
# Start a background process for each file.
in_files=$(ls ${4})
for part_file in ${in_files[@]}; do
  echo "File passed to the client is ${part_file}"
  ./"${1}" "${2}" "${3}" "${4}/${part_file}" "${5}" "${6}" "${7}" "${8}"&
  echo "Process to read file ${part_file} started with process id ${!}"
done
