#training
echo "training for query17 with large entropy weight"
for x in {1..128}; 
do bin/tpchDataLoader ~/tables-scale-10 Y Y Y N Y;
   if [ $? -ne 26 ]; then
      echo "${x}: to run query"
      bin/runQuery17;
      bin/tpchDataLoader ~/tables-scale-10 N N N Y;
   fi
done;
