
#ifndef NETSDB_TREE_RESULT_H
#define NETSDB_TREE_RESULT_H

// #define MAX_BLOCK_SIZE 2876726
// #define MAX_BLOCK_SIZE 275000
#define MAX_BLOCK_SIZE 500000
// #define MAX_BLOCK_SIZE 1000000

#include "DataTypes.h"
#include "PDBClient.h"
#include "StorageClient.h"
#include "TensorBlock2D.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
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

// PRELOAD %TreeResult%

// this class is used to store the prediction results of a single decision tree on a batch of data
namespace pdb {

class TreeResult : public Object {
  public:
    ENABLE_DEEP_COPY

    Handle<Vector<float>> data;

    int treeId;

    int batchId;

    int blockSize;

    ModelType modelType;

    TreeResult() {}

    ~TreeResult() {}

    TreeResult(int treeId, int batchId, int blockSize, ModelType modelType)
        : treeId{treeId}, batchId{batchId}, blockSize{blockSize}, modelType{modelType} {
        this->data = makeObject<Vector<float>>(blockSize, blockSize);
    }

    int &getKey() {

        return batchId;
    }

    void print() {

        std::cout << "TreeResult: " << treeId << ":" << batchId << std::endl;
    }

    TreeResult &getValue() {
        return *this;
    }

    TreeResult &operator+(TreeResult &other) {
        float *myData, *otherData;

        myData = this->data->c_ptr();
        otherData = other.data->c_ptr();

        for (int i = 0; i < blockSize; i++) {
            myData[i] += otherData[i];
        }
        return *this;
    }

    int getNumPositives() const {
        float *myData = data->c_ptr();
        int positive_count = 0;
        for (int i = 0; i < blockSize; i++) {
            positive_count += myData[i];
        }
        return positive_count;
    }

    double getBlockSum() const {
        float *myData = data->c_ptr();
        double sum = 0;
        for (int i = 0; i < blockSize; i++) {
            sum += static_cast<double>(myData[i]);
        }
        return sum;
    }

#if 1
    void postprocessing(Handle<TreeResult> &res, bool isClassification, int numTrees) {
        float *resData = res->data->c_ptr();
        float *myData = data->c_ptr();

        for (int i = 0; i < blockSize; i++) {
            resData[i] = (modelType == ModelType::RandomForest)
                             ? myData[i] / numTrees
                             : myData[i];
        }
        if (isClassification) {
            for (int i = 0; i < blockSize; i++) {
                resData[i] = (resData[i] > 0.0) ? 1.0 : 0.0;
            }
        }
    }
#else
    void postprocessing(Handle<TreeResult> &res, int numTrees = 0) {
        float *resData = res->data->c_ptr();
        float *myData = data->c_ptr();
        if ((modelType == ModelType::XGBoost) || (modelType == ModelType::LightGBM)) {
            float threshold = 0.5;
            float sigmoid_of_decision;
            for (int i = 0; i < blockSize; i++) {
                sigmoid_of_decision = 1 / (1 + exp(-1.0 * myData[i]));
                resData[i] = sigmoid_of_decision > threshold ? 1.0 : 0.0;
            }
            // Reference: https://stats.stackexchange.com/questions/395697/what-is-an-intuitive-interpretation-of-the-leaf-values-in-xgboost-base-learners
        } else {
            for (int i = 0; i < blockSize; i++) {
                resData[i] = myData[i] > numTrees / 2 ? 1.0 : 0.0;
            }
        }
    }
#endif
};
} // namespace pdb

#endif // NETSDB_TREE_RESULT_H
