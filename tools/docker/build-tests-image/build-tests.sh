#!/usr/bin/env bash

# make the build files
cmake .

# build the tests
make build-tests -j

# build the master
make pdb-server -j

# build the worker
make pdb-cluster -j

# set the permission
chmod -R 777 .