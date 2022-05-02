#!/usr/bin/env bash

# copy the binaries
cp -r ../../../bin ../cluster-image/bin

# rebuild the images
docker-compose rm
docker-compose build

# remove the binaries
rm -rf ../cluster-image/libraries
rm -rf ../cluster-image/bin

# start the cluster
docker-compose up