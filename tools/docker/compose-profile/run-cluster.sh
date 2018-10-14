#!/usr/bin/env bash

# copy the binaries
mkdir ../cluster-image/libraries
cp -r ../../../libraries/* ../cluster-image/libraries
cp -r ../../../bin/* ../cluster-image/

# rebuild the images
docker-compose rm
docker-compose build

# remove the binaries
rm -rf ../cluster-image/libraries
find ../cluster-image/ -type f -executable -delete

# start the cluster
docker-compose up