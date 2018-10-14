#!/usr/bin/env bash

# do the cleanup
rm -rf ${PDB_ROOT}/Testing
rm -rf ${PDB_ROOT}/CMakeFiles
rm -rf ${PDB_ROOT}/bin/*
rm -rf ${PDB_ROOT}/libraries/*
rm -rf ${PDB_ROOT}/logs/*
rm -rf ${PDB_ROOT}/cmake_install.cmake
rm -rf ${PDB_ROOT}/CMakeCache.txt
rm -rf ${PDB_ROOT}/CTestTestfile.cmake
rm -rf ${PDB_ROOT}/Makefile

printf "Removed!\n"