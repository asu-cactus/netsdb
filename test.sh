#!/bin/bash

./scripts/cleanup.sh ../amitab.pem &
sleep 5
./scripts/install.sh ../amitab.pem &
sleep 10

master="18.216.175.149"

# -----------------------------------------------------------------

mem=9216
threads=4
models=3

echo "> Running shared ${models} models with ${mem}"

./scripts/startMaster.sh ../amitab.pem > cluster.log 2>&1 &
sleep 2
./scripts/startWorkers.sh ../amitab.pem $master $threads $mem &
sleep 10

./bin/LoadAmazon14kData localhost 8108 Y N $models 500 500 Y 1000 14588 597540 1000 > share_${models}_1_server_${mem}_${threads}.log 2>&1
./bin/RunAmazon14k Y $models >> share_${models}_1_server_${mem}_${threads}.log 2>&1

sleep 2

echo "### 2nd Run of shared ${models} models with ${mem}"
echo "### 2nd Run of shared ${models} models with ${mem}" >> share_${models}_1_server_${mem}_${threads}.log

./bin/LoadAmazon14kData localhost 8108 N Y $models 500 500 Y 1000 14588 597540 1000 >> share_${models}_1_server_${mem}_${threads}.log 2>&1
./bin/RunAmazon14k N $models >> share_${models}_1_server_${mem}_${threads}.log 2>&1
./bin/RunAmazon14k N $models >> share_${models}_1_server_${mem}_${threads}.log 2>&1

./scripts/cleanup.sh ../amitab.pem &
sleep 5
./scripts/install.sh ../amitab.pem &
sleep 10




echo "> Running not shared ${models} models with ${mem}"

./scripts/startMaster.sh ../amitab.pem > cluster.log 2>&1 &
sleep 2
./scripts/startWorkers.sh ../amitab.pem $master $threads $mem &
sleep 10

./bin/LoadAmazon14kData localhost 8108 Y N $models 500 500 N 1000 14588 597540 1000 > no_share_${models}_1_server_${mem}_${threads}.log 2>&1
./bin/RunAmazon14k Y $models >> no_share_${models}_1_server_${mem}_${threads}.log 2>&1

sleep 2

echo "### 2nd Run of not shared ${models} models with ${mem}"
echo "### 2nd Run of not shared ${models} models with ${mem}" >> no_share_${models}_1_server_${mem}_${threads}.log

./bin/LoadAmazon14kData localhost 8108 N Y $models 500 500 N 1000 14588 597540 1000 >> no_share_${models}_1_server_${mem}_${threads}.log 2>&1
./bin/RunAmazon14k N $models >> no_share_${models}_1_server_${mem}_${threads}.log 2>&1
./bin/RunAmazon14k N $models >> no_share_${models}_1_server_${mem}_${threads}.log 2>&1

./scripts/cleanup.sh ../amitab.pem &
sleep 5
./scripts/install.sh ../amitab.pem &
sleep 10

# --------------------------------------------------------------------------------------