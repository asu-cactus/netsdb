#!/bin/bash
queryId=$1
trainingEpochs=$2


for x in {1..$trainingEpochs}
do
   ./scripts/cleanup.sh ./conf/aaa10.pem
   ./scripts/install.sh ./conf/aaa10.pem
   ./scripts/startMaster.sh ./conf/aaa10.pem
   sleep 5
   ./scripts/startWorkers.sh ./conf/aaa10.pem 172.30.4.38 12 30000
   ./bin/tpchDataLoader ~/tables_scale_10
   sleep 2
   ./bin/runQuery$queryId
   sleep 2
   ./bin/tpchDataLoader N N N N Y
done
