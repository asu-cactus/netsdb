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
#include "Node.h"
#include "Forest.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"
#include "TensorBlock2D.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"
#include "EnsembleTreeGenericUDFFloat.h"
#include "EnsembleTreeGenericUDFDouble.h"
#include "VectorFloatWriter.h"
#include "VectorDoubleWriter.h"

using namespace std;

void loadSharedLibraries(pdb::PDBClient pdbClient) {
}

int main(int argc, char *argv[]) {

    // makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

    bool createSet;

    if(argc>1) {
        if(string(argv[1]).compare("Y")==0) {
            createSet = true;
        } else {
            createSet = false;
        }
    }

    int rowNum = std::stoi(argv[2]);  //total size of input feature vectors
    int colNum = std::stoi(argv[3]);  //number of features
    int block_x = std::atoi(argv[4]); //batch size
    int block_y = colNum; //numFeatures

    string errMsg;

    bool isFloat;
    if (string(argv[5]).compare("F")==0) {
        isFloat = true;
    } else {
        isFloat = false; 
    }
    std::string forestFolderPath = std::string(argv[6]);

    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("DecisionTreeClientLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

    if(createSet == true){
        loadSharedLibraries(pdbClient);
        ff::createDatabase(pdbClient, "decisiontree");
	if (isFloat) { 
           ff::createSetGeneric<TensorBlock2D<float>>(pdbClient, "decisiontree", "inputs", "inputs", 64);
           ff::loadMatrixGeneric<TensorBlock2D<float>>(pdbClient, "decisiontree", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);
        }
   	else {
	   ff::createSetGeneric<TensorBlock2D<double>>(pdbClient, "decisiontree", "inputs", "inputs", 64);
	   ff::loadMatrixGeneric<TensorBlock2D<double>>(pdbClient, "decisiontree", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);
        }
    }else{
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }

    if (isFloat)
        ff::createSetGeneric<pdb::Vector<float>>(pdbClient, "decisiontree", "labels", "labels", 64);
    else
	ff::createSetGeneric<pdb::Vector<double>>(pdbClient, "decisiontree", "labels", "labels", 64);

    makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

    pdb::Handle<pdb::Computation> inputMatrix = nullptr;
    if (isFloat)
	 inputMatrix = pdb::makeObject<ScanUserSet<TensorBlock2D<float>>>("decisiontree", "inputs");
    else
	 inputMatrix = pdb::makeObject<ScanUserSet<TensorBlock2D<double>>>("decisiontree", "inputs");

    bool isClassificationTask = true;
    auto model_begin = chrono::high_resolution_clock::now();
    pdb::Handle<pdb::Computation> xgbGenericUDF = nullptr;

    if (isFloat)
         xgbGenericUDF = pdb::makeObject<pdb::EnsembleTreeGenericUDFFloat>(forestFolderPath, ModelType::XGBoost, isClassificationTask);
    else
	 xgbGenericUDF = pdb::makeObject<pdb::EnsembleTreeGenericUDFDouble>(forestFolderPath, ModelType::XGBoost, isClassificationTask);

    auto model_end = chrono::high_resolution_clock::now();

    xgbGenericUDF->setInput(inputMatrix);

    pdb::Handle<pdb::Computation> labelWriter = nullptr;
    if (isFloat)
         labelWriter = pdb::makeObject<pdb::VectorFloatWriter>("decisiontree", "labels");
    else
	 labelWriter = pdb::makeObject<pdb::VectorDoubleWriter>("decisiontree", "labels");
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

    bool printResult = true;
    if (printResult == true) {
          std::cout << "to print result..." << std::endl;
          int count = 0;
          if (isFloat) {

	       SetIterator<Vector<float>> result =
                   pdbClient.getSetIterator<Vector<float>>("decisiontree", "labels");
           
               for (auto a : result) {
                    count++;
                    a->print();
               }
	  } else {
	  
	       SetIterator<Vector<double>> result =
                   pdbClient.getSetIterator<Vector<double>>("decisiontree", "labels");

	       for (auto a : result) {
                    count++;
                    a->print();
               }
	  
	  }

          std::cout << "output count:" << count << "\n";
    }
    return 0;
}

#endif
