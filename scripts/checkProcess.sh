#!/bin/bash
processName=$1
echo "to check $processName"

numProcesses=0

for x in {1..10};
do
   if pgrep -x $processName > /dev/null
   then
       echo "$processName is started!"
       numProcesses=1
       break
   fi
   sleep 15
done

if [ "$numProcesses" -eq 0 ]; then
    echo "$processName hasn't started!"
    exit 1
fi

