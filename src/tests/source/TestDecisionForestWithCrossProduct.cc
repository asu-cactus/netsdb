#ifndef TEST_DECISION_FOREST_WITH_CROSSPRODUCT_CC
#define TEST_DECISION_FOREST_WITH_CROSSPRODUCT_CC

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

#include "FFMatrixUtil.h"
#include "PDBClient.h"
#include "ScanUserSet.h"
#include "SimpleFF.h"
#include "StorageClient.h"
#include "TensorBlock2D.h"
#include "Tree.h"
#include "TreeCrossProduct.h"
#include "TreeResult.h"
#include "TreeResultAggregate.h"
#include "TreeResultPostProcessing.h"
#include "WriteUserSet.h"
using namespace std;

using std::filesystem::directory_iterator;

int loadTreeData(PDBClient &pdbClient, std::string forestFolderPath, ModelType modelType, bool isClassification) {

    std::vector<std::string> treePaths;

    std::string errMsg;

    makeObjectAllocatorBlock(16 * 1024 * 1024, true);
    Handle<Vector<Handle<Tree>>> storeMe =
        makeObject<Vector<Handle<Tree>>>();

    int treeId = 0;
    for (const auto &file : directory_iterator(forestFolderPath)) {
        std::string treePath = file.path();
        try {
            Handle<Tree> tree = makeObject<Tree>(treeId, treePath, modelType);
            treePaths.push_back(treePath);
            storeMe->push_back(tree);
        } catch (pdb::NotEnoughSpace &n) {
            std::cout << "page is full when loading tree-" << treeId << std::endl;
            // send the full page to server
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
    const char *helperString =
        "Usage: \n To load data: bin/testDecisionForestWithCrossProduct Y numInstances numFeatures batch_size label_col_index, pageSizeInMB numPartitions pathToLoadDataFile(N for generating data randomly) pathToModelFolder modelType[XGBoost/RandomForest/LightGBM] numTrees dataHasMissingValues[withMissing/withoutMissing] taskType[classification/regression]\n"
        "To run the inference: bin/testDecisionForestWithCrossProduct N numInstances numFeatures batchSize labelColIndex pageSizeInMB numPartitions pathToLoadDataFile pathToModelFolder modelType[XGBoost/RandomForest/LightGBM] numTrees dataHasMissingValues[withMissing/withoutMissing] taskType[classification/regression]\n"
        "Example: \n bin/testDecisionForestWithCrossProduct Y 2200000 28 275000 0 32 1 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost 500 withoutMissing classification\n"
        "bin/testDecisionForestWithCrossProduct N 2200000 28 275000 0 32 1 model-inference/decisionTree/experiments/HIGGS.csv_test.csv model-inference/decisionTree/experiments/models/higgs_xgboost_500_8_netsdb XGBoost 500 withoutMissing classification\n";
    if ((argc <= 8) || (argc > 14)) {
        std::cout << helperString;
        exit(-1);
    }

    int rowNum = -1;
    int colNum = -1;
    int block_x = -1;
    int block_y = -1;
    int label_col_index = 0;
    string errMsg;
    string forestFolderPath;
    ModelType modelType = ModelType::XGBoost;
    int pageSize = 64;
    int numPartitions = 1;
    string dataFilePath = "";
    int numTrees = 0;
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

        label_col_index = std::atoi(argv[5]); // the index of label column

        if (argc >= 7) {
            pageSize = std::stoi(argv[6]);
        }

        if (argc >= 8) {
            numPartitions = std::stoi(argv[7]);
        }
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
        } else if (string(argv[10]).compare("RandomForest") == 0) {
            modelType = ModelType::RandomForest;
        } else if (string(argv[10]).compare("LightGBM") == 0) {
            modelType = ModelType::LightGBM;
        } else {
            std::cerr << "Unsupported model type: " << argv[10] << std::endl;
            std::cerr << helperString;
            exit(-1);
        }
    }

    if (argc >= 12) {
        numTrees = std::stoi(argv[11]);
    }

    if (argc >= 13) {
        if (std::string(argv[12]).compare("withMissing") == 0) {
            hasMissing = true;
        } else if (std::string(argv[12]).compare("withoutMissing") == 0) {
            hasMissing = false;
        } else {
            std::cerr << "Please provide an argument `withMissing` or `withoutMissing` to specify the character of the data\n";
            std::cerr << helperString;
            exit(-1);
        }
    }

    if (argc >= 14) {
        if (std::string(argv[13]).compare("classification") == 0) {
            isClassification = true;
        } else if (std::string(argv[13]).compare("regression") == 0) {
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
        // create set for data
        ff::createDatabase(pdbClient, "decisionForest");
        ff::createSetGeneric<pdb::TensorBlock2D<float>>(pdbClient, "decisionForest", "inputs", "inputs", pageSize, numPartitions, false);
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

        // create set for tree
        pdbClient.removeSet("decisionForest", "trees", errMsg);
        pdbClient.createSet<pdb::Tree>("decisionForest", "trees",
                                       errMsg, 1 * 1024 * 1024, "trees",
                                       nullptr, nullptr, false, true);

        auto model_begin = chrono::high_resolution_clock::now();
        int numTrees = loadTreeData(pdbClient, forestFolderPath, modelType, isClassification);
        auto model_end = chrono::high_resolution_clock::now();
        std::cout << numTrees << " trees loaded" << std::endl;
        std::cout << "****Model Load Time Duration: ****"
                  << std::chrono::duration_cast<std::chrono::duration<double>>(model_end - model_begin).count()
                  << " secs." << std::endl;
    } else {
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }

    if (createSet == false) {

        // create set for labels
        pdbClient.removeSet("decisionForest", "labels", errMsg);
        pdbClient.createSet<pdb::TreeResult>("decisionForest", "labels",
                                             errMsg, 1 * 1024 * 1024, "labels",
                                             nullptr, nullptr, false);

        pdb::makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

        auto begin = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numPartitions; i++) {

            pdb::Handle<pdb::Computation> inputMatrix = nullptr;

            if (numPartitions == 1)
                inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<float>>>("decisionForest", "inputs");
            else
                inputMatrix = pdb::makeObject<pdb::ScanUserSet<pdb::TensorBlock2D<float>>>("decisionForest", std::string("inputs") + std::to_string(i));

            pdb::Handle<pdb::Computation> inputTree = pdb::makeObject<pdb::ScanUserSet<Tree>>("decisionForest", "trees");

            pdb::Handle<pdb::CrossProductComp> treeCrossProduct = makeObject<TreeCrossProduct>(hasMissing);

            treeCrossProduct->setJoinType(CrossProduct);

            if (treeCrossProduct->getJoinType() == CrossProduct) {

                std::cout << "The join is CrossProduct" << std::endl;

            } else {

                std::cout << "The join is not CrossProduct" << std::endl;
            }

            pdb::Handle<pdb::Computation> treeResultAgg = makeObject<pdb::TreeResultAggregate>();

            treeResultAgg->setUsingCombiner(false);

            treeCrossProduct->setInput(0, inputTree);

            treeCrossProduct->setInput(1, inputMatrix);

            treeResultAgg->setInput(treeCrossProduct);

            pdb::Handle<pdb::Computation> treeResultPostProcessing = makeObject<pdb::TreeResultPostProcessing>(numTrees, isClassification);

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

            std::cout << i << ":****Model Inference Time Duration: ****"
                      << std::chrono::duration_cast<std::chrono::duration<double>>(exe_end - exe_begin).count()
                      << " secs." << std::endl;
        }

        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "****Overall Inference Time Duration: ****"
                  << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count()
                  << " secs." << std::endl;

        bool printResult = true;
        if (printResult == true) {
            std::cout << "to print result..." << std::endl;
            pdb::SetIterator<pdb::TreeResult> result = pdbClient.getSetIterator<pdb::TreeResult>("decisionForest", "labels");
            int count = 0;
            if (isClassification) {
                int positiveCounts = 0;
                for (auto a : result) {
                    positiveCounts += a->getNumPositives();
                    count += a->blockSize;
                }
                std::cout << "total count:" << count << "\n";
                std::cout << "positive count:" << positiveCounts << "\n";
            } else { // TODO: support larger outputs for regression tasks
                double sum = 0.0;
                for (auto a : result) {
                    sum += a->getBlockSum();
                    count += a->blockSize;
                }
                double mean = sum / count;
                std::cout << "total count:" << count << "\n";
                std::cout << "output mean:" << mean << '\n';
            }
        }
        return 0;
    }
}

#endif
