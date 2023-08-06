# Run Lachesis to Automatically Partition Data Using Self-Learning

## Rule-based
In SConstruct:

make sure below compiler flag is disabled: commented or removed:

common_env.Append(CCFLAGS='-DAPPLY_REINFORCEMENT_LEARNING')



In src/mainServer/source/MasterMain.cc:

make sure:

bool isSelfLearning = true;


and make sure:

bool trainingMode = false;

Make sure you rebuild the repository after making these changes.



## Deep Reinforcement Learning-based
In SConstruct:

make sure below compiler flag is enabled:

common_env.Append(CCFLAGS='-DAPPLY_REINFORCEMENT_LEARNING')

Make sure you rebuild the repository after making these changes.


Then start the python RL server by:

cd scripts/pangeaDeepRL

python rlServer.py



### For testing mode:

make sure in ./src/mainServer/source/MasterMain.cc:

bool isSelfLearning = true;


and make sure:

bool trainingMode = false;

Make sure you rebuild the repository after making these changes.




### For training mode:

make sure in ./src/mainServer/source/MasterMain.cc:

bool isSelfLearning = true;


and make sure:

bool trainingMode = true;

Make sure you rebuild the repository after making these changes(by using command 'scons tpchNormal').

The only way to run in training mode, is to design and develop a training process. An example of such training process is here:


#### Step 1. Determine a set of workloads for forming training traces, e.g. "TPCHQuery01", "TPCHQuery02", "TPCHQuery04". 

Before this step you need to get the TPCH dataset from TPC Benchmark. https://github.com/electrum/tpch-dbgen

Download and configure with your own environment, configure DSS_PATH to the folder path where you want this dataset be stored.

Run './dbgen -s #scaleofdata' to generate specific size of data.

Run 'bin/tpchDataLoader ~/tpchdatafilepath' to load data.

Run queries like 'bin/runQuery01', 'bin/runQuery02' to generate some workloads.


#### Step 2. Form the history by running all workloads, capture all possible candidate partitioning schemes and statistics for each partitioning predicate (lambda)

An example for this step is coded in:
src/tpch/source/tpchPrepareTraining.cc

Run 'bin/tpchPrepareTraining'


#### Step 3. Run all workloads with all potential partitioning candidates in each training environment. Store this information into a RUN_STAT table. 

An example for this step is coded in:
src/tpch/source/tpchGenTrace.cc

Run 'bin/tpchGenTrace #tpchdatafilepath #startID #endID' 


#### Step 4. Training the DRL model by randomly generates new workloads, which are arbitrary combinations of the workloads selected in Step 1.

An example of this step is coded in:
src/tpch/source/tpchTraining1.cc

Run 'bin/tpchTraining1'

Before executing the step, make sure you have started the RL server by:

cd scripts/pangeaDeepRL

python rlServer.py
