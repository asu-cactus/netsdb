
// Created by venkateshgunda on 5/10/22.
//
// Refactored by Jia to move most of the construction and prediction logic to this class and to apply more performance optimizations
//
#ifndef NETSDB_FOREST_H
#define NETSDB_FOREST_H

#include "Tree.h"

#define MAX_NUM_TREES 1600

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

// PRELOAD %Forest%

using std::filesystem::directory_iterator;

namespace pdb {

class Forest : public Object {
  public:
    ENABLE_DEEP_COPY

    Node forest[MAX_NUM_TREES][MAX_NUM_NODES_PER_TREE];
    int numTrees;
    ModelType modelType;

    Forest() {}

    Forest(ModelType type) {
        this->modelType = type;
    }

    Forest(std::string pathToFolder, ModelType modelType, bool isClassification) {
        this->constructForestFromFolder(pathToFolder, modelType, isClassification);
    }

    void constructForestFromFolder(std::string pathToFolder, ModelType modelType, bool isClassification) {

        std::vector<std::string> treePaths;

        for (const auto &file : directory_iterator(pathToFolder)) {

            treePaths.push_back(file.path());
        }

        constructForestFromPaths(treePaths, modelType, isClassification);
    }

    void constructForestFromPaths(std::vector<std::string> &treesPathIn, ModelType modelType, bool isClassification) {

        this->modelType = modelType;
        this->numTrees = treesPathIn.size();

        for (int n = 0; n < numTrees; ++n) {
            Tree::constructTreeFromPath(treesPathIn[n], modelType, &(forest[n][0]));
        }

        // STATS ABOUT THE FOREST
        std::cout << "Number of trees in the forest: " << numTrees << std::endl;
    }

    template <class InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
    T most_common(InputIt begin, InputIt end) {
        std::map<T, int> counts;
        for (InputIt it = begin; it != end; ++it) {
            if (counts.find(*it) != counts.end()) {
                ++counts[*it];
            } else {
                counts[*it] = 1;
            }
        }
        return std::max_element(counts.begin(), counts.end(), [](const std::pair<T, int> &pair1, const std::pair<T, int> &pair2) { return pair1.second < pair2.second; })
            ->first;
    }

    // Decision of an XGBoost Tree (for class-1, not class-0): sigmoid(log(previous_tree_pred(initial_value=0)/(1-previous_tree_pred(initial_value=0))) + learning_rate*(current_tree_prob))
    template <class InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
    T aggregate_decisions(InputIt begin, InputIt end) {
        // Default LR Value Source: https://xgboost.readthedocs.io/en/stable/parameter.html?highlight=0.3#parameters-for-tree-booster
        double learning_rate = 0.3;       // TODO: Hard-coding XGBoost Library default value. Change this to a parameter if modified.
        double aggregated_decision = 0.0; // Initial Prediction is 0.5, which makes log(odds) value 0. If different, the default value changes.
        double threshold = 0.5;
        for (InputIt it = begin; it != end; ++it) {
            aggregated_decision += (*it); // Should not be multiplied by Learning Rate
        }
        // Reference: https://stats.stackexchange.com/questions/395697/what-is-an-intuitive-interpretation-of-the-leaf-values-in-xgboost-base-learners
        double sigmoid_of_decision = 1 / (1 + exp(-1.0 * aggregated_decision)); // Sigmoid of the aggregated decision is the final output
        return sigmoid_of_decision > threshold ? 1.0 : 0.0;                     // Delaying Class Assignment. Class Labels as per RF Code is 1.0 and 2.0
    }

    // TODO: Call this function instead of Individual Compute Functions. Instantiate the Class with the Model Type
    template <class InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
    T compute_result(InputIt begin, InputIt end) {
        switch (modelType) {
        case ModelType::RandomForest:
            return most_common(begin, end);
        case ModelType::XGBoost:
            return aggregate_decisions(begin, end);
        case ModelType::LightGBM:
            return aggregate_decisions(begin, end);
        default: // RF is default
            return most_common(begin, end);
        }
    }
    template <class T>
    pdb::Handle<pdb::Vector<T>> predictWithMissingValues(Handle<TensorBlock2D<T>> &in) {
        return predict<T, true>(in);
    }

    template <class T>
    pdb::Handle<pdb::Vector<T>> predictWithoutMissingValues(Handle<TensorBlock2D<T>> &in) {
        return predict<T, false>(in);
    }
    template <class T, bool hasMissing>
    inline pdb::Handle<pdb::Vector<T>> predict(Handle<TensorBlock2D<T>> &in) { // TODO: Change all Double References to Float

        // get the input features matrix information
        uint32_t inNumRow = in->getRowNums();
        uint32_t inNumCol = in->getColNums();

        T *inData = in->getValue().rawData->c_ptr();

        // set the output matrix
        pdb::Handle<pdb::Vector<T>> resultMatrix = pdb::makeObject<pdb::Vector<T>>(inNumRow, inNumRow);
        std::vector<T> thisResultMatrix(numTrees);

        T *outData = resultMatrix->c_ptr();

        T inputValue;
        Node treeNode;
        int featureStartIndex;
        int curIndex;

        for (int i = 0; i < inNumRow; i++) {
            featureStartIndex = i * inNumCol;
            for (int j = 0; j < numTrees; j++) {
                //  inference
                //  pass the root node of the tree
                Node *tree = forest[j];
                treeNode = tree[0];
                while (treeNode.isLeaf == false) {

                    if (inData[featureStartIndex + treeNode.indexID] <= treeNode.returnClass) {
                        treeNode = tree[treeNode.leftChild];
                    } else {
                        treeNode = tree[treeNode.rightChild];
                    }
                }
                thisResultMatrix[j] = treeNode.returnClass;
            }
            T voteResult = compute_result(thisResultMatrix.begin(), thisResultMatrix.end());
            outData[i] = voteResult;
        }
        return resultMatrix;
    }
};
} // namespace pdb

#endif // NETSDB_FOREST_H
