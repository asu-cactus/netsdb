#ifndef TEST_XGBOOST_GENERIC_UDF_CC
#define TEST_XGBOOST_GENERIC_UDF_CC

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <future>
#include <thread>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <cstdio>
#include <exception>
#include <cassert>

#include "PDBClient.h"
#include "StorageClient.h"
#include "PDBClient.h"
#include "BaselineNode.h"
#include "MyPrediction.h"
#include "TreeNode.h"
#include "Tree.h"
#include "RandomForest.h"
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"
#include "XGBoostGenericUDF.h"

using namespace std;

void loadSharedLibraries() {
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
    ff::loadLibrary(pdbClient, "libraries/libFFMatrixPartitioner.so");
    ff::loadLibrary(pdbClient, "libraries/libTreeNode.so"); // TODO: Create the file behind this Library
    ff::loadLibrary(pdbClient, "libraries/libTree.so"); // TODO: Create the file behind this Library
    ff::loadLibrary(pdbClient, "libraries/libForest.so"); // TODO: Create the file behind this Library
    ff::loadLibrary(pdbClient, "libraries/libXGBoostGenericUDF.so"); // TODO: Create the file behind this Library
}

int main(int argc, char *argv[]) {

    // makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

    bool createSet;

    if(argc>1) {
        if(argv[1].compare("Y")==0) {
            createSet = true;
        } else {
            createSet = false;
        }
    }

    int rowNum = std::stoi(argv[2]);
    int colNum = std::stoi(argv[3]);
    int block_x = std::stoi(argv[4]);
    int block_y = std::stoi(argv[5]);

    string errMsg;

    string task_type = string(argv[6]);  // TODO: Consider an Enum for this

    pdb::Vector<string> treePath;

    for(int i=7; i < argc; i++) {
        treePath.push_back(string(argv[i]));
    }

    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("DecisionTreeClientLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

    if(createSetOrNot == true){
        loadSharedLibraries();

        ff::createDatabase(pdbClient, "decisiontree");
        ff::createSet(pdbClient, "decisiontree", "inputs", "inputs", 64);
        ff::loadMatrix(pdbClient, "decisiontree", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);
    }else{
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }

    ff::createSet(pdbClient, "decisiontree", "labels", "labels", 64);

    makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

    pdb::Handle<pdb::Computation> inputMatrix = pdb::makeObject<FFMatrixBlobkScanner>("decisiontree", "inputs");

    auto model_begin = chrono::high_resolution_clock::now();
    pdb::Handle<pdb::Computation> xgbGenericUDF = pdb::makeObject<decisionTree::XGBoostGenericUDF>(treePath, task_type);
    auto model_end = chrono::high_resolution_clock::now();

    xgbGenericUDF->setInput(inputMatrix);

    pdb::Handle<pdb::Computation> labelWriter = nullptr;
    labelWriter = pdb::makeObject<FFMatrixWriter>("decisiontree", "labels");
    labelWriter->setInput(xgbGenericUDF);

    bool materializeHash = true;
    auto exe_begin = std::chrono::high_resolution_clock::now();
    if (!pdbClient.executeComputations(errMsg, "decisiontree", materializeHash, labelWriter)) {
        cout << "Computation failed. Message was: " << errMsg << "\n";
        exit(1);
    }
    auto exe_end = std::chrono::high_resolution_clock::now();


    std::cout << "****XGBoost UDF Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(exe_end - exe_begin).count()
                                                                        << " secs." << std::endl;
    std::cout << "****XGBoost UDF Load Model Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(model_end-model_begin).count()
                                                                        << " secs." << std::endl;

    //verify the results
    std::cout << "To print the status" << std::endl;
    ff::print_stats(pdbClient, "decisiontree", "labels");
    std::cout << "To print the results" << std::endl;
    ff::print(pdbClient, "decisiontree", "labels");

    // TODO: Correctness of the results is not measured. Confusion Matrix, Accuracy etc.

    return 0;
}