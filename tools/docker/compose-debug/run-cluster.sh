#!/usr/bin/env bash

# copy the binaries
cp -r ../../../bin ../cluster-image/bin

# set environment variable
export PDB_DIR=$(readlink -f ../../../)

# rebuild the images
docker-compose rm
docker-compose build

# remove the binaries
rm -rf ../cluster-image/libraries
rm -rf ../cluster-image/bin

# start the cluster
docker-compose up