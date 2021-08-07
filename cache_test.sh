#!/bin/bash

# import numpy as np
# read=np.arange(0.00, 4.26, 0.1)
# write=np.arange(0.00, 6.30, 0.1)
# write[::-1].sort()
# read[::-1].sort()
# "({})".format(" ".join([f"{x:.2f}" for x in write]))
# "({})".format(" ".join([f"{x:.2f}" for x in read]))

# write_costs=(6.20 6.10 6.00 5.90 5.80 5.70 5.60 5.50 5.40 5.30 5.20 5.10 5.00 4.90 4.80 4.70 4.60 4.50 4.40 4.30 4.20 4.10 4.00 3.90 3.80 3.70 3.60 3.50 3.40 3.30 3.20 3.10 3.00 2.90 2.80 2.70 2.60 2.50 2.40 2.30 2.20 2.10 2.00 1.90 1.80 1.70 1.60 1.50 1.40 1.30 1.20 1.10 1.00 0.90 0.80 0.70 0.60 0.50 0.40 0.30 0.20 0.10)
# read_costs=(4.20 4.10 4.00 3.90 3.80 3.70 3.60 3.50 3.40 3.30 3.20 3.10 3.00 2.90 2.80 2.70 2.60 2.50 2.40 2.30 2.20 2.10 2.00 1.90 1.80 1.70 1.60 1.50 1.40 1.30 1.20 1.10 1.00 0.90 0.80 0.70 0.60 0.50 0.40 0.30 0.20 0.10)

# write_costs=(6.20)
# read_costs=(3.70 3.60 3.50 3.40 3.30 3.20 3.10 3.00 2.90 2.80 2.70 2.60 2.50 2.40 2.30 2.20 2.10 2.00 1.90 1.80 1.70 1.60 1.50 1.40 1.30 1.20 1.10 1.00 0.90 0.80 0.70 0.60 0.50 0.40 0.30 0.20 0.10)

write_costs=(6.10 6.00 5.90 5.80 5.70 5.60 5.50 5.40 5.30 5.20 5.10 5.00 4.90 4.80 4.70 4.60 4.50 4.40 4.30 4.20 4.10 4.00 3.90 3.80 3.70 3.60 3.50 3.40 3.30 3.20 3.10 3.00 2.90 2.80 2.70 2.60 2.50 2.40 2.30 2.20 2.10 2.00 1.90 1.80 1.70 1.60 1.50 1.40 1.30 1.20 1.10 1.00 0.90 0.80 0.70 0.60 0.50 0.40 0.30 0.20 0.10)
read_costs=(4.20 4.10 4.00 3.90 3.80 3.70 3.60 3.50 3.40 3.30 3.20 3.10 3.00 2.90 2.80 2.70 2.60 2.50 2.40 2.30 2.20 2.10 2.00 1.90 1.80 1.70 1.60 1.50 1.40 1.30 1.20 1.10 1.00 0.90 0.80 0.70 0.60 0.50 0.40 0.30 0.20 0.10)

models=3
mem=9216
threads=4
master="172.31.19.173"

for write_cost in "${write_costs[@]}"
do
    for read_cost in "${read_costs[@]}"
    do

        echo "> Running workload with WRITE_COST=${write_cost} and READ_COST=${read_cost}"

        export SHARED_WRITE_COST=${write_cost}
        export SHARED_READ_COST=${read_cost}
        scons && scons libFFTest

        sleep 60
        # ./test.sh ${write_cost} ${read_cost} > t1_${write_cost}_${read_cost}.log 2>&1


        ./scripts/cleanup.sh ../amitab.pem &
        sleep 10
        ./scripts/install.sh ../amitab.pem &
        sleep 10

        echo "> Running shared ${models} models with ${mem}, write cost ${write_cost}, read cost ${read_cost}"

        ./scripts/startMaster.sh ../amitab.pem > cluster.log 2>&1
        sleep 10
        ./scripts/startWorkers.sh ../amitab.pem $master $threads $mem
        sleep 10

        ./bin/LoadAmazon14kData localhost 8108 Y N $models 500 500 Y 1000 14588 597540 1000 N > share_${models}_1_server_${mem}_${threads}_${write_cost}_${read_cost}.log 2>&1
        ./bin/RunAmazon14k Y $models N >> share_${models}_1_server_${mem}_${threads}_${write_cost}_${read_cost}.log 2>&1

        sleep 2

        ./scripts/roll_server_logs.sh ../amitab.pem ${mem} ${threads} ${models} 1 share ${write_cost} ${read_cost} &

        sleep 5

        echo "### 2nd Run of shared ${models} models with ${mem}, write cost ${write_cost}, read cost ${read_cost}"
        echo "### 2nd Run of shared ${models} models with ${mem}, write cost ${write_cost}, read cost ${read_cost}" >> share_${models}_1_server_${mem}_${threads}_${write_cost}_${read_cost}.log

        ./bin/LoadAmazon14kData localhost 8108 N Y $models 500 500 Y 1000 14588 597540 1000 Y >> share_${models}_1_server_${mem}_${threads}_${write_cost}_${read_cost}.log 2>&1
        ./bin/RunAmazon14k N $models Y >> share_${models}_1_server_${mem}_${threads}_${write_cost}_${read_cost}.log 2>&1
        ./bin/RunAmazon14k N $models Y >> share_${models}_1_server_${mem}_${threads}_${write_cost}_${read_cost}.log 2>&1


        sleep 2

        ./scripts/roll_server_logs.sh ../amitab.pem ${mem} ${threads} ${models} 2 share ${write_cost} ${read_cost} &

        sleep 5

        ./scripts/cleanup.sh ../amitab.pem &
        sleep 10

        fuser -k 8108/tcp
        kill -9 $(lsof -t -i:8108)
        sleep 61

    done
done