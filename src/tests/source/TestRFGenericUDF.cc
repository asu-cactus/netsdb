#ifndef TEST_RF_GENERIC_UDF_CC
#define TEST_RF_GENERIC_UDF_CC

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
#include "RFGenericUDF.h"

using namespace std;

// 1st parameter is the program itself
// 2nd parameter gives whether to create set or not
// 3rd - 6th parameter gives the number of rowNum, colNum, block_x and block_y
// testing for higgs, an example for 3rd - 6th parameters are: 2000,28,100,28
// 7th parameter specifies whether to classification ("C") or regression ("R")
// Starting with the 8th parameter, each parameter will represent one path of a tree. The following is 2 running examples.
// $bin/rfgenericUDF Y 2000 28 100 28 C /home/jiaqingchen/netsdb/graphs/higgs/higgs_0.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_1.txt
// $bin/rfgenericUDF N 2000 90 100 90 R /home/jiaqingchen/netsdb/graphs/year/year_0.txt /home/jiaqingchen/netsdb/graphs/year/year_1.txt
int main(int argc, char *argv[]) {

    makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

    bool createSetOrNot = true;

    if ( argc > 1 ) {
        if ( strcmp( argv[1], "Y") == 0 ) {
            createSetOrNot = true;
        } else {
            createSetOrNot = false;
        }
    }

	int rowNum = std::stoi(argv[2]);
	int colNum = std::stoi(argv[3]);
	int block_x = std::stoi(argv[4]);
	int block_y = std::stoi(argv[5]);

	string errMsg;

    // "C" represents classification and "R" represents regression
    std::string type = std::string(argv[6]);

    pdb::Vector<std::string> treePath;

    for(int i = 7; i < argc; i++){
        treePath.push_back(std::string(argv[i]));
    }
    
    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("DecisionTreeClientLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

    if(createSetOrNot == true){
        ff::createDatabase(pdbClient, "decisiontree");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
        ff::loadLibrary(pdbClient, "libraries/libFFMatrixPartitioner.so");
        ff::createSet(pdbClient, "decisiontree", "inputs", "inputs", 64);
        ff::loadLibrary(pdbClient, "libraries/libTreeNode.so");
        ff::loadLibrary(pdbClient, "libraries/libTree.so");
        ff::loadLibrary(pdbClient, "libraries/libRandomForest.so");
        ff::loadLibrary(pdbClient, "libraries/libRFGenericUDF.so");
        ff::loadMatrix(pdbClient, "decisiontree", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);
    }else{
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }

    auto begin = std::chrono::high_resolution_clock::now();
    
    //we need create the output set each time
    ff::createSet(pdbClient, "decisiontree", "labels", "labels", 64);

    makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

    
    pdb::Handle<pdb::Computation> inputMatrix = pdb::makeObject<FFMatrixBlockScanner>("decisiontree", "inputs");

    //std::cout << "To make object of decision tree UDF shared libraries" << std::endl;
    pdb::Handle<pdb::Computation> rfgenericUDF = pdb::makeObject<decisiontree::RFGenericUDF>(treePath, type);

    
    rfgenericUDF->setInput(inputMatrix);

    //std::cout << "To set the Computation" << std::endl;
    pdb::Handle<pdb::Computation> labelWriter = nullptr;
    labelWriter = pdb::makeObject<FFMatrixWriter>("decisiontree", "labels");
    labelWriter->setInput(rfgenericUDF);

    bool materializeHash = true;
    //std::cout << "To run the Computation" << std::endl;
    auto exe_begin = std::chrono::high_resolution_clock::now();
    if (!pdbClient.executeComputations(errMsg, "decisiontree", materializeHash, labelWriter)) {
        cout << "Computation failed. Message was: " << errMsg << "\n";
        exit(1);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "****Random Forest UDF Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(end - exe_begin).count()
              << " secs." << std::endl;

    std::cout << "****Random Forest UDF End-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count()
              << " secs." << std::endl;

    //verify the results
    std::cout << "To print the status" << std::endl;
    ff::print_stats(pdbClient, "decisiontree", "labels");
    std::cout << "To print the results" << std::endl;
    ff::print(pdbClient, "decisiontree", "labels");
    return 0;
}

#endif
