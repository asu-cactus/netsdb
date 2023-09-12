#ifndef TEST_XGBOOST_GENERIC_UDF_CC
#define TEST_XGBOOST_GENERIC_UDF_CC

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

#include "EnsembleTreeGenericUDFDouble.h"
#include "EnsembleTreeGenericUDFFloat.h"
#include "EnsembleTreeUDFDouble.h"
#include "EnsembleTreeUDFFloat.h"
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
        "Usage: \n To load data: bin/testDecisionForest Y numInstances numFeatures batch_size label_col_index isFloat[F/D] isGraphOrArray[G/A] pageSizeInMB numPartitions pathToLoadDataFile(N for generating data randomly) pathToModelFolder modelType[XGBoost/RandomForest/LightGBM] dataHasMissingValues[withMissing/withoutMissing] taskType[classification/regression]\n"
        "To run the inference: bin/testDecisionForest N numInstances numFeatures batchSize label_col_index isFloat[F/D] isGraphOrArray [G/A] pageSizeInMB numPartitions pathToLoadDataFile pathToModelFolder modelType[XGBoost/RandomForest/LightGBM] dataHasMissingValues[withMissing/withoutMissing] taskType[classification/regression]\n"
        "Example: \n bin/testDecisionForest Y 2200000 28 275000 0 F A 32 1 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost XGBoost withoutMissing classification\n"
        "bin/testDecisionForest N 2200000 28 275000 0 F A 32 1 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost withoutMissing classification\n";
    bool createSet;

    if ((argc <= 8) || (argc > 15)) {

        std::cerr << helperString;
        exit(-1);
    }

    int rowNum = -1;
    int colNum = -1;
    int block_x = -1;
    int block_y = -1;
    int label_col_index = 0;
    bool isFloat = true;
    bool isGraph = true;
    string errMsg;
    string forestFolderPath;
    ModelType modelType = ModelType::XGBoost;
    int pageSize = 64;
    int numPartitions = 1;
    string dataFilePath = "";
    bool hasMissing = false;
    bool isClassification = true;

    if (argc >= 8) {

        if (string(argv[1]).compare("Y") == 0) {
            createSet = true;
        } else {
            createSet = false;
        }

        rowNum = std::stoi(argv[2]);  // total size of input feature vectors
        colNum = std::stoi(argv[3]);  // number of features
        block_x = std::atoi(argv[4]); // batch size
        block_y = colNum;             // numFeatures

        label_col_index = std::atoi(argv[5]);

        if (string(argv[6]).compare("F") == 0) {
            isFloat = true;
        } else {
            isFloat = false;
        }

        if (string(argv[7]).compare("G") == 0) {
            isGraph = true;
        } else {
            isGraph = false;
        }
    }

    if (argc >= 9) {
        pageSize = std::stoi(argv[8]);
    }

    if (argc > 10) {
        numPartitions = std::stoi(argv[9]);
    }

    if (argc >= 11) {
        dataFilePath = std::string(argv[10]);
    }

    if (argc >= 12) {
        forestFolderPath = std::string(argv[11]);
    }

    if (argc >= 13) {
        if (string(argv[12]).compare("XGBoost") == 0) {
            modelType = ModelType::XGBoost;
        } else if (string(argv[12]).compare("LightGBM") == 0) {
            modelType = ModelType::LightGBM;
        } else if (string(argv[12]).compare("RandomForest") == 0) {
            modelType = ModelType::RandomForest;
        } else {
            std::cout << "Unsupported model type: " << argv[12] << std::endl;
            std::cout << helperString;
            exit(-1);
        }
    }

    if (argc >= 14) {
        if (std::string(argv[13]).compare("withMissing") == 0) {
            hasMissing = true;
        } else if (std::string(argv[13]).compare("withoutMissing") == 0) {
            hasMissing = false;
        } else {
            std::cout << "Please provide an argument `withMissing` or `withoutMissing` to specify the character of the data\n";
            std::cout << helperString;
            exit(-1);
        }
    }

    if (argc >= 15) {
        if (std::string(argv[14]).compare("classification") == 0) {
            isClassification = true;
        } else if (std::string(argv[14]).compare("regression") == 0) {
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
        if (isFloat) {
            ff::createSetGeneric<pdb::TensorBlock2D<float>>(pdbClient, "decisionForest", "inputs", "inputs", pageSize, numPartitions);
            if (dataFilePath.compare("N") == 0) {
                if (numPartitions == 1)
                    ff::loadMatrixGeneric<pdb::TensorBlock2D<float>>(pdbClient, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);
                else {

                    std::cout << "Currently we only support single partition for randomly generated input data" << std::endl;
                    exit(1);
                }

            } else {
                ff::loadMatrixGenericFromFile<pdb::TensorBlock2D<float>>(pdbClient, dataFilePath, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, label_col_index, errMsg, 4 * pageSize, numPartitions);
            }
        } else {
            ff::createSetGeneric<pdb::TensorBlock2D<double>>(pdbClient, "decisionForest", "inputs", "inputs", pageSize, numPartitions);
            if (dataFilePath.compare("N") == 0) {

                if (numPartitions == 1) {
                    ff::loadMatrixGeneric<pdb::TensorBlock2D<double>>(pdbClient, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, false, false, errMsg);
                } else {
                    std::cout << "Currently we only support single partition for randomly generated input data" << std::endl;
                    exit(1);
                }
            } else {
                ff::loadMatrixGenericFromFile<pdb::TensorBlock2D<double>>(pdbClient, dataFilePath, "decisionForest", "inputs", rowNum, colNum, block_x, block_y, label_col_index, errMsg, 4 * pageSize, numPartitions);
            }
        }
    } else {
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }

    if (createSet == false) {
        if (isFloat)
            ff::createSetGeneric<pdb::Vector<float>>(pdbClient, "decisionForest", "labels", "labels", 64);
        else
            ff::createSetGeneric<pdb::Vector<double>>(pdbClient, "decisionForest", "labels", "labels", 64);

        pdb::makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

        auto begin = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numPartitions; i++) {

            pdb::Handle<pdb::Computation> inputMatrix = nullptr;

            if (isFloat) {
                if (numPartitions == 1)
                    inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<float>>>("decisionForest", "inputs");
                else
                    inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<float>>>("decisionForest", std::string("inputs") + std::to_string(i));
            } else {
                if (numPartitions == 1)
                    inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<double>>>("decisionForest", "inputs");
                else
                    inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<double>>>("decisionForest", std::string("inputs") + std::to_string(i));
            }
            auto model_begin = chrono::high_resolution_clock::now();

            pdb::Handle<pdb::Computation> decisionForestUDF = nullptr;

            if (isFloat) {
                if (isGraph)
                    decisionForestUDF = pdb::makeObject<pdb::EnsembleTreeUDFFloat>(forestFolderPath, modelType, isClassification);
                else
                    decisionForestUDF = pdb::makeObject<pdb::EnsembleTreeGenericUDFFloat>(forestFolderPath, modelType, isClassification, hasMissing);
            } else {
                if (isGraph)
                    decisionForestUDF = pdb::makeObject<pdb::EnsembleTreeUDFDouble>(forestFolderPath, modelType, isClassification);
                else
                    decisionForestUDF = pdb::makeObject<pdb::EnsembleTreeGenericUDFDouble>(forestFolderPath, modelType, isClassification, hasMissing);
            }
            auto model_end = chrono::high_resolution_clock::now();

            decisionForestUDF->setInput(inputMatrix);

            pdb::Handle<pdb::Computation> labelWriter = nullptr;

            if (isFloat)
                labelWriter = pdb::makeObject<pdb::VectorFloatWriter>("decisionForest", "labels");
            else
                labelWriter = pdb::makeObject<pdb::VectorDoubleWriter>("decisionForest", "labels");

            labelWriter->setInput(decisionForestUDF);

            bool materializeHash = false;
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

            if (isFloat) {
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
                } else { // TODO: support larger outputs for regression tasks
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

            } else {

                pdb::SetIterator<pdb::Vector<double>> result =
                    pdbClient.getSetIterator<pdb::Vector<double>>("decisionForest", "labels");
            }
        }
    }
    return 0;
}

#endif
