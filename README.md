# netsDB - An UDF-Centric Analytics Database


## Credits

netsDB is built on top of an earlier version of the open source project PlinyCompute (https://github.com/riceplinygroup/plinycompute).


## Building

Requirements:  
Software:

clang/llvm/build-essential

sudo apt install clang llvm build-essential

scons http://scons.org/

bison

flex

LLVM/clang++3.8 or above

Snappy	libsnappy1v5, libsnappy-dev

GSL	     libgsl-dev

Boost	libboost-dev, libboost-program-options-dev, libboost-filesystem-dev, libboost-system-dev


OS: Ubuntu-16, MacOS, Ubuntu-20

Run: scons 

## Run netsDB on local

python scripts/startPseudoCluster.py #numThreads #sharedMemPoolSize (MB)


## Cleanup netsDB Data on local

scripts/cleanupNode.sh


## Run netsDB on a Cluster 

### Configuration

Step (1) Download netsDB code from github to the Master server, configure PDB_HOME to be the github repository. For example, you can:

     - edit ~/.bashrc, and add following to that file: export PDB_HOME=/home/ubuntu/netsDB
     Here /home/ubuntu/netsDB should be replaced by the path to the github repository

Step (2) Configure PDB_INSTALL to be the location that you want pangea to be installed at on the workers.  For example, you might add the following to .basrc:

     export PDB_INSTALL=/disk1/netsDB
     Here /disk1/netsDB should be the path to the directory where you want the binary and code to be copied to, and where data will be stored.
     Make sure that the user that runs the program has authorization to read/write/create in this directory

After Step (1) and (2), run `source ~/.bashrc' in shell to make sure these variables are set. 

Step (3) Find one or more servers as your Workers, make sure those Workers can be accessed by Master through network and vice versa, and also make sure you have only one PEM file to log on to all slaves. Then add only IPs of those slaves to the file: $PDB_HOME/conf/serverlist. For example, my serverlist looks like following:

10.134.96.184
10.134.96.153  

Step (4) On the master server, install the cluster by run:
     
     scripts/install.sh $pem_file


### Start the cluster

On the Master server:

Step (1)

cd $PDB_HOME
scripts/startMaster.sh $pem_file

wait for the scripts to return (see something like "master is started!" in the end), and move to  step 2.3:

Step (2) : run following command:   
 
cd $PDB_HOME
scripts/startWorkers.sh $pem_file $MasterIPAddress $ThreadNumber (optional, default is 4)  $SharedMemSize (optional, unit MB, default is 4096)

wait for the scripts to return (see something like "servers are started!" in the end).


After the cluster is started, you can run your application.


### Stop Cluster
cd $PDB_HOME
scripts/stopWorkers.sh $pem_file


### Soft Reboot Cluster (restart cluster with all data kept)
cd $PDB_HOME
scripts/stopWorkers.sh $pem_filex
scripts/startMaster.sh $pem_file
scripts/startWorkers.sh $pem_file $MasterIPAddress $ThreadNum $SharedMemoryPoolSize


### Upgrade Cluster (for developers and testers upgrade binaries and restart cluster with all data kept)
cd $PDB_HOME
scripts/stopWorkers.sh $pem_file
scripts/upgrade.sh $pem_file
scripts/startMaster.sh $pem_file
scripts/startWorkers.sh $pem_file $MasterIPAddress $ThreadNum $SharedMemoryPoolSize


### Cleanup Catalog and Storage data
You can cleanup all catalog and storage data by running following command in master

cd $PDB_HOME
scripts/cleanup.sh $pem_file


### Environment Variables:


(1) PDB_SSH_OPTS

by default, it is defined to be "-o StrictHostKeyChecking=no"

(2) PDB_SSH_FOREGROUND

if you define it to non empty like "y" or "yes", it will run as before and bring all output to your ssh terminal;

by default, it is not defined, and it will run in background using nohup, which means it will not be interrupted by ssh.

# Examples of Running DNN model inferences from netsDB

[Model Inferences](model-inference/README.md)

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
