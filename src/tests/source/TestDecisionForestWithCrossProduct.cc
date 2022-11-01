#ifndef TEST_DECISION_FOREST_WITH_CROSSPRODUCT_CC
#define TEST_DECISION_FOREST_WITH_CROSSPRODUCT_CC

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
#include "Tree.h"
#include "TreeResult.h"
#include "TreeResultPostProcessing.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"
#include "TreeResultAggregate.h"
#include "TreeCrossProduct.h"
using namespace std;


using std::filesystem::directory_iterator;

int loadTreeData(PDBClient& pdbClient, std::string forestFolderPath, ModelType modelType) {

	std::vector<std::string> treePaths;

	std::string errMsg;

        makeObjectAllocatorBlock(16 * 1024 * 1024, true);
	Handle<Vector<Handle<Tree>>> storeMe =
                    makeObject<Vector<Handle<Tree>>>();

	int treeId = 0;
        for (const auto & file : directory_iterator(forestFolderPath)) {
	     std::string treePath = file.path(); 	
	     try {
                 Handle<Tree> tree = makeObject<Tree>(treeId, treePath, modelType);             
                 treePaths.push_back(treePath);
		 storeMe->push_back(tree);
             } catch (pdb::NotEnoughSpace& n) {
		 std::cout << "page is full when loading tree-" << treeId << std::endl;
	         //send the full page to server
                 if (!pdbClient.sendData<Tree>(
                            std::pair<std::string, std::string>("trees", "decisionForest"),
                            storeMe,
                            errMsg)) {
                        std::cout << "Failed to send data to dispatcher server" << std::endl;
                 }
		 makeObjectAllocatorBlock(16 * 1024 * 1024, true);
                 storeMe = makeObject<Vector<Handle<Tree>>>();
                 Handle<Tree> tree = makeObject<Tree>(treeId, treePath, modelType);
                 storeMe->push_back(tree);
	     }
	     treeId++;
        }

	if (!pdbClient.sendData<Tree>(
                            std::pair<std::string, std::string>("trees", "decisionForest"),
                            storeMe,
                            errMsg)) {
                        std::cout << "Failed to send data to dispatcher server" << std::endl;
                 }

	pdbClient.flushData(errMsg);

	return treeId;

    }


int main(int argc, char *argv[]) {


    bool createSet;

    if ((argc <= 5)||(argc > 10)) {
    
        std::cout << "Usage: \n To load data: bin/testDecisionForestWithCrossProduct Y numInstances numFeatures batch_size pageSizeInMB pathToLoadDataFile(N for generating data randomly) pathToModelFolder modelType[XGBoost/RandomForest]\n To run the inference: bin/testDecisionForestWithCrossProduct N numInstances numFeatures batchSize pageSizeInMB pathToLoadDataFile pathToModelFolder modelType[XGBoost/RandomForest]\n Example: \n bin/testDecisionForestWithCrossProduct Y 2200000 28 275000 32 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost\n bin/testDecisionForestWithCrossProduct N 2200000 28 275000 32 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost\n";
        exit(-1);
    }

    int rowNum = -1;
    int colNum = -1;
    int block_x = -1;
    int block_y = -1;
    string errMsg;
    string forestFolderPath;
    ModelType modelType = ModelType::XGBoost;
    int pageSize = 64;
    string dataFilePath = "";
    int numTrees = 0;

    if(argc >= 5) {

        if(string(argv[1]).compare("Y")==0) {
            createSet = true;
        } else {
            createSet = false;
        }
    
        rowNum = std::stoi(argv[2]);  //total size of input feature vectors
        colNum = std::stoi(argv[3]);  //number of features
        block_x = std::atoi(argv[4]); //batch size
        block_y = colNum; //numFeatures

    }

    if (argc >= 6) {
        pageSize = std::stoi(argv[5]);
    }

    if (argc >= 7) {
        dataFilePath = std::string(argv[6]);
    }

    if (argc >= 8) {
        forestFolderPath = std::string(argv[7]);
    }

    if (argc >= 9) {
        if (string(argv[8]).compare("XGBoost") == 0) {
            modelType = ModelType::XGBoost;
        } else if (string(argv[8]).compare("RandomForest") == 0) {
            modelType = ModelType::RandomForest;
        } else if (string(argv[8]).compare("LightGBM") == 0) {
	    modelType = ModelType::LightGBM;
	} else {
            std::cerr << "Unsupported model type: " << argv[8] << std::endl;
	    exit(-1);
        }
    }

    if (argc >= 10) {
        numTrees = std::stoi(argv[9]);
    }


    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("DecisionForestClientLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);


    if(createSet == true){
        //create set for data
        ff::createDatabase(pdbClient, "decisionForest");
        ff::createSetGeneric<pdb::TensorBlock2D<float>>(pdbClient, "decisionForest", "inputs", "inputs", pageSize);
        if (dataFilePath.compare("N") == 0) {
	    ff::loadMatrixGeneric<pdb::TensorBlock2D<float>>(pdbClient, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);		   
	} else {
	    ff::loadMatrixGenericFromFile<pdb::TensorBlock2D<float>>(pdbClient, dataFilePath, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, errMsg, 128);
	}

	//create set for tree
        pdbClient.removeSet("decisionForest", "trees", errMsg);
        pdbClient.createSet<pdb::Tree>("decisionForest", "trees",
                                           errMsg, 8*1024*1024, "trees",
                                           nullptr, nullptr, false);
	
        auto model_begin = chrono::high_resolution_clock::now();
	int numTrees = loadTreeData(pdbClient, forestFolderPath, modelType);
        auto model_end = chrono::high_resolution_clock::now();
	std::cout << numTrees << " trees loaded" << std::endl;
        std::cout << "****Model Load Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(model_end-model_begin).count()
                                                                        << " secs." << std::endl;
    } else {
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }
   
    if (createSet == false) {

	//create set for labels
        pdbClient.removeSet("decisionForest", "labels", errMsg);
        pdbClient.createSet<pdb::Tree>("decisionForest", "labels",
                                           errMsg, 64*1024*1024, "labels",
                                           nullptr, nullptr, false);

        pdb::makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

        pdb::Handle<pdb::Computation> inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<float>>>("decisionForest", "inputs");

	pdb::Handle<pdb::Computation> inputTree = pdb::makeObject<pdb::ScanUserSet<Tree>>("decisionForest", "trees");


	//if ((modelType == ModelType::XGBoost) || (modelType == ModelType::LightGBM)) {

            pdb::Handle<pdb::CrossProductComp> treeCrossProduct = makeObject<TreeCrossProduct>();

            treeCrossProduct->setJoinType(CrossProduct);

            if (treeCrossProduct->getJoinType() == CrossProduct) {
	
	       std::cout << "The join is CrossProduct" << std::endl;

	    } else {
	
	       std::cout << "The join is not CrossProduct" << std::endl;
	
	    }

	    pdb::Handle<pdb::Computation> treeResultAgg = makeObject<pdb::TreeResultAggregate>();
        
	
	    treeCrossProduct->setInput(0, inputTree);

	    treeCrossProduct->setInput(1, inputMatrix);


            treeResultAgg->setInput(treeCrossProduct);

            pdb::Handle<pdb::Computation> treeResultPostProcessing = makeObject<pdb::TreeResultPostProcessing>( numTrees );
	
            treeResultPostProcessing->setInput(treeResultAgg);

	    pdb::Handle<pdb::Computation> resultWriter = makeObject<WriteUserSet<TreeResult>>("decisionForest", "labels");

	    resultWriter->setInput(treeResultPostProcessing);

            bool materializeHash = true;

            auto exe_begin = std::chrono::high_resolution_clock::now();
            if (!pdbClient.executeComputations(errMsg, "decisionForest", materializeHash, resultWriter)) {
                cout << "Computation failed. Message was: " << errMsg << "\n";
                exit(1);
            }
            auto exe_end = std::chrono::high_resolution_clock::now();

            std::cout << "****Model Inference Time Duration: ****"
                << std::chrono::duration_cast<std::chrono::duration<double>>(exe_end - exe_begin).count()
                                                                        << " secs." << std::endl;
       
       bool printResult = true;
       if (printResult == true) {
           std::cout << "to print result..." << std::endl;
           int count = 0;
           int positive_count = 0;

           pdb::SetIterator<pdb::TreeResult> result =
           pdbClient.getSetIterator<pdb::TreeResult>("decisionForest", "labels");

           for (auto a : result) {
               positive_count += a->getNumPositives();
               count += a->blockSize;
           }
           std::cout << "total count:" << count << "\n";
           std::cout << "positive count:" << positive_count << "\n";
       }
       return 0;
    }
}

#endif
