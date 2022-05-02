#!/usr/bin/env bash

# copy configurations
cp -r ../../../scripts ./scripts
cp -r ../../../conf ./conf

# build the pdb image
docker build . -t dimitrijejankov/pdb

# remove the copied configurations
rm -rf ./scripts
rm -rf ./conf