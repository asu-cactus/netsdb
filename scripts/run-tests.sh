#!/usr/bin/env bash

# grab the root directory
export PDB_ROOT=$(pwd)

# build the tests
printf "Building the tests!\n"
sleep 1
cd ${PDB_ROOT}/tools/docker/build-tests-image
./build-command.sh

# run the tests
printf "Running the tests!\n"
sleep 1
cd ${PDB_ROOT}/tools/docker/compose
./run-cluster.sh