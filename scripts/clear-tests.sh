#!/usr/bin/env bash

# grab the root directory
export PDB_ROOT=$(pwd)

# prompt if we should do the cleanup
cd ${PDB_ROOT}/tools/docker/build-tests-image

printf "Do you want to remove the test build data?\n"

options=("Yes" "No")
select opt in "${options[@]}"
do
    case $opt in
        "Yes")
            ./clean-tests.sh; cd ${PDB_ROOT}; exit
            ;;
        "No")
            cd ${PDB_ROOT}; exit
            ;;
        *) echo invalid option;;
    esac
done