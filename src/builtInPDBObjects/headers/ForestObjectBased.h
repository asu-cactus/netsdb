
#ifndef NETSDB_FOREST_OBJECT_BASED_H
#define NETSDB_FOREST_OBJECT_BASED_H

#include "Forest.h"
#include "PDBClient.h"
#include "StorageClient.h"
#include "TensorBlock2D.h"
#include "TreeNodeObjectBased.h"
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

// PRELOAD %ForestObjectBased%

using std::filesystem::directory_iterator;

namespace pdb {

class ForestObjectBased : public Object {
  public:
    ENABLE_DEEP_COPY

    Handle<Vector<Handle<TreeNodeObjectBased>>> roots = makeObject<Vector<Handle<TreeNodeObjectBased>>>();
    int numTrees;
    ModelType modelType;

    ForestObjectBased() {}

    ForestObjectBased(ModelType type) {
        this->modelType = type;
    }

    ForestObjectBased(Handle<Forest> forest) {
        this->numTrees = forest->numTrees;
        this->modelType = forest->modelType;
        for (int i = 0; i < numTrees; i++) {
            Handle<TreeNodeObjectBased> curRoot = makeObject<TreeNodeObjectBased>();
            constructNode(curRoot, forest, i, 0);
            roots->push_back(curRoot);
        }
    }

    void constructNode(Handle<TreeNodeObjectBased> curNode, Handle<Forest> forest, int treeId, int nodeId) {
        Node correspondingNode = forest->forest[treeId][nodeId];
        curNode->threshold = correspondingNode.threshold;
        if (!correspondingNode.isLeaf) {
            curNode->index = correspondingNode.indexID;

            int leftChildId = correspondingNode.leftChild;
            if (leftChildId > 0) {
                curNode->leftChild = makeObject<TreeNodeObjectBased>();
                constructNode(curNode->leftChild, forest, treeId, leftChildId);
            } else {

                curNode->leftChild = nullptr;
            }

            int rightChildId = correspondingNode.rightChild;
            if (rightChildId > 0) {
                curNode->rightChild = makeObject<TreeNodeObjectBased>();
                constructNode(curNode->rightChild, forest, treeId, rightChildId);
            } else {

                curNode->rightChild = nullptr;
            }
        } else {

            curNode->leftChild = nullptr;
            curNode->rightChild = nullptr;
        }
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
        return sigmoid_of_decision > threshold ? 2.0 : 1.0;                     // Delaying Class Assignment. Class Labels as per RF Code is 1.0 and 2.0
    }

    // TODO: Call this function instead of Individual Compute Functions. Instantiate the Class with the Model Type
    template <class InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
    T compute_result(InputIt begin, InputIt end) {
        switch (modelType) {
        case ModelType::RandomForest:
            return most_common(begin, end);
        case ModelType::XGBoost:
            return aggregate_decisions(begin, end);
            //                case ModelType::LightGBM:
            //                    return 0.0f;
        default: // RF is default
            return most_common(begin, end);
        }
    }

    void print() {

        Handle<TreeNodeObjectBased> treeNode;
        std::cout << numTrees << " trees in total" << std::endl;
        for (int i = 0; i < numTrees; i++) {
            std::cout << "the " << i << "-th tree:" << std::endl;
            treeNode = (*roots)[i];
            treeNode->print();
        }
    }

    template <class T>
    pdb::Handle<pdb::Vector<T>> predict(Handle<TensorBlock2D<T>> &in) {

        // get the input features matrix information
        uint32_t inNumRow = in->getRowNums();
        uint32_t inNumCol = in->getColNums();

        T *inData = in->getValue().rawData->c_ptr();

        // set the output matrix
        pdb::Handle<pdb::Vector<T>> resultMatrix = pdb::makeObject<pdb::Vector<T>>(inNumRow, inNumRow);
        std::vector<T> thisResultMatrix(numTrees);

        T *outData = resultMatrix->c_ptr();

        T inputValue;

        Handle<TreeNodeObjectBased> treeNode = nullptr;
        int featureStartIndex;
        int curIndex;
        for (int i = 0; i < inNumRow; i++) {
            featureStartIndex = i * inNumCol;
            for (int j = 0; j < numTrees; j++) {
                treeNode = (*roots)[j];
                while (treeNode->leftChild != nullptr) {
                    if (inData[featureStartIndex + treeNode->index] <= treeNode->threshold) {
                        treeNode = treeNode->leftChild;
                    } else {
                        if (treeNode->rightChild != nullptr)
                            treeNode = treeNode->rightChild;
                        else
                            break;
                    }
                }
                thisResultMatrix[j] = treeNode->leafValue;
            }
            T voteResult = compute_result(thisResultMatrix.begin(), thisResultMatrix.end());
            outData[i] = voteResult;
        }

        return resultMatrix;
    }
};
} // namespace pdb

#endif // NETSDB_FOREST_OBJECT_BASED_H
