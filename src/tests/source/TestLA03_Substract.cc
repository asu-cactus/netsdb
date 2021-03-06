#ifndef TEST_LA_03_CC
#define TEST_LA_03_CC


// by Binhang, May 2017
// to test matrix substract implemented by join;

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "LAScanMatrixBlockSet.h"
#include "LAWriteMatrixBlockSet.h"
#include "LASillySubstractJoin.h"
//#include "BuiltInMatrixBlock.h"
#include "MatrixBlock.h"
#include "Set.h"
#include "DataTypes.h"
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>


using namespace pdb;
int main(int argc, char* argv[]) {
    bool printResult = true;
    bool clusterMode = false;
    std::cout
        << "Usage: #printResult[Y/N] #clusterMode[Y/N] #dataSize[MB] #masterIp #SubstractData[Y/N]"
        << std::endl;
    if (argc > 1) {
        if (strcmp(argv[1], "N") == 0) {
            printResult = false;
            std::cout << "You successfully disabled printing result." << std::endl;
        } else {
            printResult = true;
            std::cout << "Will print result." << std::endl;
        }
    } else {
        std::cout << "Will print result. If you don't want to print result, you can Substract N as "
                     "the first parameter to disable result printing."
                  << std::endl;
    }

    if (argc > 2) {
        if (strcmp(argv[2], "Y") == 0) {
            clusterMode = true;
            std::cout << "You successfully set the test to run on cluster." << std::endl;
        } else {
            clusterMode = false;
        }
    } else {
        std::cout << "Will run on local node. If you want to run on cluster, you can Substract any "
                     "character as the second parameter to run on the cluster configured by "
                     "$PDB_HOME/conf/serverlist."
                  << std::endl;
    }

    int blockSize = 64;  // by default we Substract 64MB data
    if (argc > 3) {
        blockSize = atoi(argv[3]);
    }
    blockSize = 64;  // Force it to be 64 by now.


    std::cout << "To Substract data with size: " << blockSize << "MB" << std::endl;

    std::string masterIp = "localhost";
    if (argc > 4) {
        masterIp = argv[4];
    }
    std::cout << "Master IP Substractress is " << masterIp << std::endl;

    bool whetherToSubstractData = true;
    if (argc > 5) {
        if (strcmp(argv[5], "N") == 0) {
            whetherToSubstractData = false;
        }
    }

    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");

    PDBClient pdbClient(
            8108, masterIp,
            clientLogger,
            false,
            true);

    CatalogClient catalogClient(
            8108,
            masterIp,
            clientLogger);

    string errMsg;

    if (whetherToSubstractData == true) {
        // Step 1. Create Database and Set
        // now, register a type for user data
        // TODO: once sharedLibrary is supported, Substract this line back!!!
        pdbClient.registerType("libraries/libMatrixBlock.so", errMsg);

        // now, create a new database
        if (!pdbClient.createDatabase("LA03_db", errMsg)) {
            cout << "Not able to create database: " + errMsg;
            exit(-1);
        } else {
            cout << "Created database.\n";
        }

        // now, create the first matrix set in that database
        if (!pdbClient.createSet<MatrixBlock>("LA03_db", "LA_input_set1", errMsg)) {
            cout << "Not able to create set: " + errMsg;
            exit(-1);
        } else {
            cout << "Created set.\n";
        }

        // now, create the first matrix set in that database
        if (!pdbClient.createSet<MatrixBlock>("LA03_db", "LA_input_set2", errMsg)) {
            cout << "Not able to create set: " + errMsg;
            exit(-1);
        } else {
            cout << "Created set.\n";
        }


        // Step 2. Substract data
        int matrixRowNums = 4;
        int matrixColNums = 4;
        int blockRowNums = 10;
        int blockColNums = 5;

        int total = 0;

        // Substract Matrix 1
        pdb::makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
        pdb::Handle<pdb::Vector<pdb::Handle<MatrixBlock>>> storeMatrix1 =
            pdb::makeObject<pdb::Vector<pdb::Handle<MatrixBlock>>>();

        for (int i = 0; i < matrixRowNums; i++) {
            for (int j = 0; j < matrixColNums; j++) {
                pdb::Handle<MatrixBlock> myData =
                    pdb::makeObject<MatrixBlock>(i, j, blockRowNums, blockColNums);
                // Foo initialization
                for (int ii = 0; ii < blockRowNums; ii++) {
                    for (int jj = 0; jj < blockColNums; jj++) {
                        (*(myData->getRawDataHandle()))[ii * blockColNums + jj] =
                            i + j + ii + jj + 1.0;
                    }
                }

                std::cout << "New block: " << total << std::endl;
                myData->print();
                storeMatrix1->push_back(myData);
                total++;
            }
        }

        if (!pdbClient.sendData<MatrixBlock>(
                std::pair<std::string, std::string>("LA_input_set1", "LA03_db"),
                storeMatrix1,
                errMsg)) {
            std::cout << "Failed to send data to dispatcher server" << std::endl;
            return -1;
        }
        PDB_COUT << total << " MatrixBlock data sent to dispatcher server~~" << std::endl;
        // to write back all buffered records
        pdbClient.flushData(errMsg);

        // Substract Matrix 2
        total = 0;
        pdb::makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
        pdb::Handle<pdb::Vector<pdb::Handle<MatrixBlock>>> storeMatrix2 =
            pdb::makeObject<pdb::Vector<pdb::Handle<MatrixBlock>>>();

        for (int i = 0; i < matrixRowNums; i++) {
            for (int j = 0; j < matrixColNums; j++) {
                pdb::Handle<MatrixBlock> myData =
                    pdb::makeObject<MatrixBlock>(i, j, blockRowNums, blockColNums);
                // Foo initialization
                for (int ii = 0; ii < blockRowNums; ii++) {
                    for (int jj = 0; jj < blockColNums; jj++) {
                        (*(myData->getRawDataHandle()))[ii * blockColNums + jj] = i + j + ii + jj;
                    }
                }
                std::cout << "New block: " << total << std::endl;
                myData->print();
                storeMatrix2->push_back(myData);
                total++;
            }
        }

        if (!pdbClient.sendData<MatrixBlock>(
                std::pair<std::string, std::string>("LA_input_set2", "LA03_db"),
                storeMatrix2,
                errMsg)) {
            std::cout << "Failed to send data to dispatcher server" << std::endl;
            return -1;
        }
        PDB_COUT << total << " MatrixBlock data sent to dispatcher server~~" << std::endl;
        // to write back all buffered records
        pdbClient.flushData(errMsg);
    }
    // now, create a new set in that database to store output data

    PDB_COUT << "to create a new set for storing output data" << std::endl;
    if (!pdbClient.createSet<MatrixBlock>("LA03_db", "LA_diff_set", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit(-1);
    } else {
        cout << "Created set.\n";
    }

    // Step 3. To execute a Query
    // for allocations
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // register this query class
    pdbClient.registerType("libraries/libLASillySubstractJoin.so", errMsg);
    pdbClient.registerType("libraries/libLAScanMatrixBlockSet.so", errMsg);
    pdbClient.registerType("libraries/libLAWriteMatrixBlockSet.so", errMsg);



    Handle<Computation> myMatrixSet1 = makeObject<LAScanMatrixBlockSet>("LA03_db", "LA_input_set1");
    Handle<Computation> myMatrixSet2 = makeObject<LAScanMatrixBlockSet>("LA03_db", "LA_input_set2");

    Handle<Computation> mySubstractJoin = makeObject<LASillySubstractJoin>();
    mySubstractJoin->setInput(0, myMatrixSet1);
    mySubstractJoin->setInput(1, myMatrixSet2);

    Handle<Computation> mySumWriteSet = makeObject<LAWriteMatrixBlockSet>("LA03_db", "LA_diff_set");
    mySumWriteSet->setInput(mySubstractJoin);

    auto begin = std::chrono::high_resolution_clock::now();

    if (!pdbClient.executeComputations(errMsg, mySumWriteSet)) {
        std::cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }
    std::cout << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    // std::cout << "Time Duration: " <<
    //      std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << " ns." <<
    //      std::endl;

    std::cout << std::endl;
    // print the resuts
    if (printResult == true) {
        std::cout << "to print result..." << std::endl;

        SetIterator<MatrixBlock> input1 =
            pdbClient.getSetIterator<MatrixBlock>("LA03_db", "LA_input_set1");
        std::cout << "Input Matrix 1:" << std::endl;
        int countIn1 = 0;
        for (auto a : input1) {
            countIn1++;
            std::cout << countIn1 << ":";
            a->print();
            std::cout << std::endl;
        }
        std::cout << "Matrix1 input block nums:" << countIn1 << "\n";

        SetIterator<MatrixBlock> input2 =
            pdbClient.getSetIterator<MatrixBlock>("LA03_db", "LA_input_set2");
        std::cout << "Input Matrix 2:" << std::endl;
        int countIn2 = 0;
        for (auto a : input2) {
            countIn2++;
            std::cout << countIn2 << ":";
            a->print();
            std::cout << std::endl;
        }
        std::cout << "Matrix2 input block nums:" << countIn2 << "\n";


        SetIterator<MatrixBlock> result =
            pdbClient.getSetIterator<MatrixBlock>("LA03_db", "LA_diff_set");
        std::cout << "Substract query results: " << std::endl;
        int countOut = 0;
        for (auto a : result) {
            countOut++;
            std::cout << countOut << ":";
            a->print();

            std::cout << std::endl;
        }
        std::cout << "Diff output count:" << countOut << "\n";
    }

    if (clusterMode == false) {
        // and delete the sets
        pdbClient.deleteSet("LA03_db", "LA_diff_set");
    } else {
        if (!pdbClient.removeSet("LA03_db", "LA_diff_set", errMsg)) {
            cout << "Not able to remove set: " + errMsg;
            exit(-1);
        } else {
            cout << "Removed set.\n";
        }
    }
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }
    std::cout << "Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
}

#endif
