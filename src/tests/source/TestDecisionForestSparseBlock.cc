#ifndef TEST_DECISION_FOREST_SPARSE_CC
#define TEST_DECISION_FOREST_SPARSE_CC

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "EnsembleTreeGenericUDFSparseBlock.h"
#include "FFMatrixUtil.h"
#include "Forest.h"
#include "PDBClient.h"
#include "ScanUserSet.h"
#include "SimpleFF.h"
#include "StorageClient.h"
#include "TensorBlock2D.h"
#include "VectorDoubleWriter.h"
#include "VectorFloatWriter.h"
#include "WriteUserSet.h"

using namespace std;

int main(int argc, char *argv[]) {

    const char *helperString =
        "Usage: \n To load data: bin/testDecisionForestSparse Y numInstances numFeatures pageSizeInMB numPartitions pathToLoadDataFile pathToModelFolder modelType[XGBoost/RandomForest/LightGBM] taskType[classification/regression]\n"
        "To run the inference: bin/testDecisionForestSparse N numInstances numFeatures pageSizeInMB numPartitions pathToLoadDataFile pathToModelFolder modelType[XGBoost/RandomForest/LightGBM] taskType[classification/regression]\n"
        "Example: \n bin/testDecisionForestSparse Y 2200000 28 32 1 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost classification\n"
        "bin/testDecisionForestSparse N 2200000 28 32 1 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost classification\n";
    bool createSet;

    if ((argc < 6) || (argc > 10)) {

        std::cerr << helperString;
        exit(-1);
    }

    int rowNum = -1;
    int colNum = -1;
    string errMsg;
    string forestFolderPath;
    ModelType modelType = ModelType::XGBoost;
    int pageSize = 64;
    int numPartitions = 1;
    string dataFilePath = "";
    bool isClassification = true;

    if (argc >= 6) {

        if (string(argv[1]).compare("Y") == 0) {
            createSet = true;
        } else {
            createSet = false;
        }

        rowNum = std::stoi(argv[2]); // total size of input feature vectors
        colNum = std::stoi(argv[3]); // number of features
        pageSize = std::stoi(argv[4]);
        numPartitions = std::stoi(argv[5]);
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
        } else if (string(argv[8]).compare("LightGBM") == 0) {
            modelType = ModelType::LightGBM;
        } else if (string(argv[8]).compare("RandomForest") == 0) {
            modelType = ModelType::RandomForest;
        } else {
            std::cout << "Unsupported model type: " << argv[8] << std::endl;
            std::cout << helperString;
            exit(-1);
        }
    }

    if (argc >= 10) {
        if (std::string(argv[9]).compare("classification") == 0) {
            isClassification = true;
        } else if (std::string(argv[9]).compare("regression") == 0) {
            isClassification = false;
        } else {
            std::cout << "Please provide an argument `classification` or `regression` to specify type of the task\n";
            std::cout << helperString;
            exit(-1);
        }
    }

    string masterIp = "localhost";
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("DecisionForestClientLog");
    pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
    pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

    if (createSet == true) {
        ff::createDatabase(pdbClient, "decisionForest");
        ff::createSetGeneric<pdb::Vector<pdb::Handle<pdb::Map<int, float>>>>(pdbClient, "decisionForest", "inputs", "inputs", pageSize, numPartitions);
        ff::loadMapFromSVMFile(pdbClient, dataFilePath, "decisionForest", "inputs", rowNum, colNum, errMsg, 2 * pageSize, numPartitions);
    } else {
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }

    if (createSet == false) {
        ff::createSetGeneric<pdb::Vector<float>>(pdbClient, "decisionForest", "labels", "labels", 64);

        pdb::makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

        auto begin = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numPartitions; i++) {

            pdb::Handle<pdb::Computation> inputScanner = nullptr;

            if (numPartitions == 1)
                inputScanner = pdb::makeObject<pdb::ScanUserSet<pdb::Vector<pdb::Handle<pdb::Map<int, float>>>>>("decisionForest", "inputs");
            else
                inputScanner = pdb::makeObject<pdb::ScanUserSet<pdb::Vector<pdb::Handle<pdb::Map<int, float>>>>>("decisionForest", std::string("inputs") + std::to_string(i));

            auto model_begin = chrono::high_resolution_clock::now();

            pdb::Handle<pdb::Computation> decisionForestUDFSparseBlock = pdb::makeObject<pdb::EnsembleTreeGenericUDFSparseBlock>(forestFolderPath, modelType, isClassification);

            auto model_end = chrono::high_resolution_clock::now();

            decisionForestUDFSparseBlock->setInput(inputScanner);

            pdb::Handle<pdb::Computation> labelWriter = pdb::makeObject<pdb::VectorFloatWriter>("decisionForest", "labels");

            labelWriter->setInput(decisionForestUDFSparseBlock);

            bool materializeHash = true;
            auto exe_begin = std::chrono::high_resolution_clock::now();
            if (!pdbClient.executeComputations(errMsg, "decisionForest", materializeHash, labelWriter)) {
                cout << "Computation failed. Message was: " << errMsg << "\n";
                exit(1);
            }
            auto exe_end = std::chrono::high_resolution_clock::now();

            std::cout << i << ":****UDF Execution Time Duration: ****"
                      << std::chrono::duration_cast<std::chrono::duration<double>>(exe_end - exe_begin).count()
                      << " secs." << std::endl;
            std::cout << i << ":****UDF Load Model Time Duration: ****"
                      << std::chrono::duration_cast<std::chrono::duration<double>>(model_end - model_begin).count()
                      << " secs." << std::endl;
        }

        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "****Overall Inference Time Duration: ****"
                  << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count()
                  << " secs." << std::endl;

        bool printResult = true;
        if (printResult == true) {
            std::cout << "to print result..." << std::endl;

            pdb::SetIterator<pdb::Vector<float>> result = pdbClient.getSetIterator<pdb::Vector<float>>("decisionForest", "labels");
            int count = 0;
            if (isClassification) {
                int positiveCounts = 0;
                for (auto a : result) {
                    for (int i = 0; i < a->size(); i++) {
                        count++;
                        positiveCounts += (*a)[i];
                    }
                }
                std::cout << "output count:" << count << "\n";
                std::cout << "positive count:" << positiveCounts << "\n";
            }

            else { // TODO: support larger outputs for regression tasks
                double sum = 0.0;
                for (auto a : result) {
                    for (int i = 0; i < a->size(); i++) {
                        count++;
                        sum += (*a)[i];
                    }
                }
                double mean = sum / count;
                std::cout << "output count:" << count << "\n";
                std::cout << "output mean:" << mean << '\n';
            }
        }
    }
    return 0;
}

#endif
