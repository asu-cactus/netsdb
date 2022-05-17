#!/usr/bin/env bash

# build the test builder image
docker build . -t dimitrijejankov/pdb-test-builder

echo ${PDB_ROOT}

# run the image
docker run -v ${PDB_ROOT}:/pdb dimitrijejankov/pdb-test-builder