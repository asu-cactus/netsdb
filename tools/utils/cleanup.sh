#!/bin/bash
DATA_DIRS="pdbRoot"
CALALOG_DIAR="catalog"

echo "Removing old folders and killing PDB processes. "


# remove old files 
if [ "$1" = "y" ]; then 

echo "Removing old files ..."
rm -rf $DATA_DIRS $CALALOG_DIAR; 

fi;

echo "Killing Process pdbServer"

for KILLPID in `ps ax | grep '[p]dbServer' | awk ' { print $1;}'`; do 
  kill -9 $KILLPID;
done

echo "sleep 2 sec and kill again pdbServer processes"

sleep 2


for KILLPID in `ps ax | grep '[p]dbServer' | awk ' { print $1;}'`; do 
  kill -9 $KILLPID;
done

echo "Done!"




