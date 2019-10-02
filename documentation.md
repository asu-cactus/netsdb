# Documentation for Gram task, linear regression and matrix multiplication experiments in Pangea


### Results
| Experiment name       | No self learning | Self learning - 1 | Self learning - 2 | Self learning - 3 | Self learning - 1 (2) | Self learning - 2 (2) |
|-----------------------|:----------------:|:-----------------:|-------------------|-------------------|-----------------------|-----------------------|
| Gram multiplication   | 41.2693 secs.    | 63.0345 secs      | 22.9496 secs      | 22.7832 secs.     |                       |                       |
| Linear regression     | 83.4468 secs.    | 102.471 secs      | 43.9066 secs      | 43.9436 secs      |                       |                       |
| Matrix multiplication | 42.2086 secs.    | 41.2701 secs.     | 16.563 secs.      |                   | 11.406 secs.          | 11.4764 secs.         |
|                       |                  |                   |                   |                   |                       |                       |

### Initial steps
1. Ensure self learning is on by
   1. In SConstruct:

      make sure below compiler flag is disabled: commented or removed:

          common_env.Append(CCFLAGS='-DAPPLY_REINFORCEMENT_LEARNING')
    
   2.  In src/mainServer/source/MasterMain.cc:

        make sure:

            bool isSelfLearning = true; (if you want to use self learning)
            (or)
            bool isSelfLearning = false (if you want to disable self learning)

        and make sure:

            bool trainingMode = false;


2. Run
    scons libLATest

3. Delete pangea/selfLearningDB folder to remove previous history
4. Run the following commands:
   
       scripts/cleanup.sh ~/pass
       scripts/install.sh ~/pass
       scripts/startMaster.sh ~/pass
       scripts/startWorkers.sh ~/pass 10.128.0.7 4 15000


## Gram task
To generate data:

    python ./src/linearAlgebraDSL/TestDataGenerator/GramTestDataGenerator.py 200000 1000 1000 1000
Where the first four parameters are matrix row size, matrix column size, block row size, block column size

To run query:

    ./bin/testLA21_Instance N Y 256 localhost ./src/linearAlgebraDSL/DSLSamples/Task01_Gram_200000_1000_1000_1000.pdml 

## Linear algebra 
To generate data:

    python ./src/linearAlgebraDSL/TestDataGenerator/L2TestDataGenerator.py 200000 1000 1000 1000
Where the first four parameters are matrix row size, matrix column size, block row size, block column size

To run query:

    ./bin/testLA21_Instance N Y 256 localhost ./src/linearAlgebraDSL/DSLSamples/Task02_L2_200000_1000_1000_1000.pdml 

## Matrix multiplication
To delete data:

    bin/testLA04_Multiply N N Y

To generate data:

    bin/testLA04_Multiply Y N N

To run query:

    bin/testLA04_Multiply N Y N
