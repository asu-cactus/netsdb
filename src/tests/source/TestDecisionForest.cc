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
#include "TensorBlock2D.h"
#include "Forest.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"
#include "EnsembleTreeGenericUDFFloat.h"
#include "EnsembleTreeGenericUDFDouble.h"
#include "EnsembleTreeUDFFloat.h"
#include "EnsembleTreeUDFDouble.h"
#include "VectorFloatWriter.h"
#include "VectorDoubleWriter.h"

using namespace std;


int main(int argc, char *argv[]) {


    bool createSet;

    if ((argc <= 7)||(argc > 11)) {
    
        std::cout << "Usage: \n To load data: bin/testDecisionForest Y numInstances numFeatures batch_size isFloat[F/D] isGraphOrArray[G/A] pageSizeInMB pathToLoadDataFile(N for generating data randomly)\n To run the inference: bin/testDecisionForest N numInstances numFeatures batchSize isFloat[F/D] isGraphOrArray [G/A] pageSizeInMB pathToLoadDataFile pathToModelFolder modelType[XGBoost/RandomForest]\n Example: \n bin/testDecisionForest Y 2200000 28 275000 F A 32 model-inference/decisionTree/experiments/HIGGS.csv_test.csv\n bin/testDecisionForest N 2200000 28 275000 F A 32 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost\n";
        exit(-1);
    }

    int rowNum = -1;
    int colNum = -1;
    int block_x = -1;
    int block_y = -1;
    bool isFloat = true;
    bool isGraph = true;
    string errMsg;
    string forestFolderPath;
    ModelType modelType = ModelType::XGBoost;
    int pageSize = 64;
    string dataFilePath = "";

    if(argc >= 7) {

        if(string(argv[1]).compare("Y")==0) {
            createSet = true;
        } else {
            createSet = false;
        }
    
        rowNum = std::stoi(argv[2]);  //total size of input feature vectors
        colNum = std::stoi(argv[3]);  //number of features
        block_x = std::atoi(argv[4]); //batch size
        block_y = colNum; //numFeatures

        if (string(argv[5]).compare("F")==0) {
            isFloat = true;
        } else {
            isFloat = false; 
        }

	 if (string(argv[6]).compare("G")==0) {
            isGraph = true;
        } else {
            isGraph = false;
        }
    }

    if (argc >= 8) {
        pageSize = std::stoi(argv[7]);
    }

    if (argc >= 9) {
        dataFilePath = std::string(argv[8]);
    }

    if (argc >= 10) {
        forestFolderPath = std::string(argv[9]);
    }

    if (argc >= 11) {
        if (string(argv[10]).compare("XGBoost") == 0) {
            modelType = ModelType::XGBoost;
	} else if (string(argv[10]).compare("LightGBM") == 0) {
            modelType = ModelType::LightGBM;
        } else if (string(argv[10]).compare("RandomForest") == 0) {
            modelType = ModelType::RandomForest;
        } else {
            std::cerr << "Unsupported model type: " << argv[10] << std::endl;
	    exit(-1);
        }
    }


    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("DecisionForestClientLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

    if(createSet == true){
        ff::createDatabase(pdbClient, "decisionForest");
	if (isFloat) { 
           ff::createSetGeneric<pdb::TensorBlock2D<float>>(pdbClient, "decisionForest", "inputs", "inputs", pageSize);
           if (dataFilePath.compare("N") == 0) {
	       ff::loadMatrixGeneric<pdb::TensorBlock2D<float>>(pdbClient, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);		   
	   } else {
	       ff::loadMatrixGenericFromFile<pdb::TensorBlock2D<float>>(pdbClient, dataFilePath, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, errMsg, 128);
	   }
	}
   	else {
	   ff::createSetGeneric<pdb::TensorBlock2D<double>>(pdbClient, "decisionForest", "inputs", "inputs", pageSize);
	   if (dataFilePath.compare("N") == 0) {
               ff::loadMatrixGeneric<pdb::TensorBlock2D<double>>(pdbClient, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);
           } else {
	       ff::loadMatrixGenericFromFile<pdb::TensorBlock2D<double>>(pdbClient, dataFilePath, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, errMsg, 128);
           }
	}
    } else{
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }
   
    if (createSet == false) {
        if (isFloat) 
            ff::createSetGeneric<pdb::Vector<float>>(pdbClient, "decisionForest", "labels", "labels", 64);
        else
	    ff::createSetGeneric<pdb::Vector<double>>(pdbClient, "decisionForest", "labels", "labels", 64);

        pdb::makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

        pdb::Handle<pdb::Computation> inputMatrix = nullptr;

        if (isFloat)
	    inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<float>>>("decisionForest", "inputs");
        else
	    inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<double>>>("decisionForest", "inputs");

        bool isClassificationTask = true;
        auto model_begin = chrono::high_resolution_clock::now();

        pdb::Handle<pdb::Computation> decisionForestUDF = nullptr;

        if (isFloat) {
            if (isGraph) 
                decisionForestUDF = pdb::makeObject<pdb::EnsembleTreeUDFFloat>(forestFolderPath, modelType, isClassificationTask);
	    else
                decisionForestUDF = pdb::makeObject<pdb::EnsembleTreeGenericUDFFloat>(forestFolderPath, modelType, isClassificationTask);
        }
    	else {
	    if (isGraph)
		decisionForestUDF = pdb::makeObject<pdb::EnsembleTreeUDFDouble>(forestFolderPath, modelType, isClassificationTask);
	    else
	        decisionForestUDF = pdb::makeObject<pdb::EnsembleTreeGenericUDFDouble>(forestFolderPath, modelType, isClassificationTask);
        }
        auto model_end = chrono::high_resolution_clock::now();

        decisionForestUDF->setInput(inputMatrix);

        pdb::Handle<pdb::Computation> labelWriter = nullptr;

        if (isFloat)
            labelWriter = pdb::makeObject<pdb::VectorFloatWriter>("decisionForest", "labels");
        else
	    labelWriter = pdb::makeObject<pdb::VectorDoubleWriter>("decisionForest", "labels");
        
	labelWriter->setInput(decisionForestUDF);

        bool materializeHash = true;
        auto exe_begin = std::chrono::high_resolution_clock::now();
        if (!pdbClient.executeComputations(errMsg, "decisionForest", materializeHash, labelWriter)) {
            cout << "Computation failed. Message was: " << errMsg << "\n";
            exit(1);
        }
        auto exe_end = std::chrono::high_resolution_clock::now();


        std::cout << "****UDF Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(exe_end - exe_begin).count()
                                                                        << " secs." << std::endl;
        std::cout << "****UDF Load Model Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(model_end-model_begin).count()
                                                                        << " secs." << std::endl;

        bool printResult = true;
        if (printResult == true) {
            std::cout << "to print result..." << std::endl;
            int count = 0;
            int positive_count = 0;
	    if (isFloat) {

		  pdb::SetIterator<pdb::Vector<float>> result =
                   pdbClient.getSetIterator<pdb::Vector<float>>("decisionForest", "labels");
           
                  for (auto a : result) {
		      for (int i = 0; i < a->size(); i++) {
                         count++;
		         positive_count += (*a)[i];
		      } 
                  }
                  std::cout << "output count:" << count << "\n";
	          std::cout << "positive count:" << positive_count << "\n";
	     } else {
	  
		  pdb::SetIterator<pdb::Vector<double>> result =
                   pdbClient.getSetIterator<pdb::Vector<double>>("decisionForest", "labels");

	    }

        }

    }
    return 0;
}

#endif
