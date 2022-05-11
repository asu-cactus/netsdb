//
// Created by venkateshgunda on 5/10/22.
//

#ifndef NETSDB_FOREST_H
#define NETSDB_FOREST_H

#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "TreeNode.h"
#include "Tree.h"
#include "PDBClient.h"
#include "StorageClient.h"

//using namespace pdb;

namespace decisiontree {

    class Forest: public Object {
    public:
        ENABLE_DEEP_COPY

        enum struct ModelType { RandomForest, XGBoost, LightGBM }; // TODO: Find a better place

        pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> forest;
        int numTrees;
        ModelType modelType;

        Forest() {}
        ~Forest() {}

        Forest(pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> forestIn, ModelType type=ModelType::RandomForest) {
            this->forest = forestIn;
            this->numTree = forestIn.size();
            this->modelType = type;
        }

        static bool compareByNodeID(const decisiontree::Node &a, const decisiontree::Node &b){
            return a.nodeID < b.nodeID;
        }

        pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> get_forest() {
            return forest;
        }

        void set_forest(pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> forestIn) {
            forest = forestIn;
            numTree = forestIn.size();
        }

        template<class InputIt, class T = typename std::iterator_traits<InputIt>::value_type> T most_common(InputIt begin, InputIt end){
            std::map<T, int> counts;
            for (InputIt it = begin; it != end; ++it) {
                if (counts.find(*it) != counts.end()) {
                    ++counts[*it];
                }
                else {
                    counts[*it] = 1;
                }
            }
            return std::max_element(counts.begin(), counts.end(),[] (const std::pair<T, int>& pair1, const std::pair<T, int>& pair2) {
                return pair1.second < pair2.second;})->first;
        }

        // Decision of an XGBoost Tree (for class-1, not class-0): sigmoid(log(previous_tree_pred(initial_value=0)/(1-previous_tree_pred(initial_value=0))) + learning_rate*(val2/(val1+val2)))
        static template<class InputIt, class T = typename std::iterator_traits<InputIt>::value_type> T aggregate_decisions(InputIt begin, InputIt end){
//          std::map<T, int> counts; // TODO: This function code needs to be re-written for XGBoost
            // Default LR Value Source: https://xgboost.readthedocs.io/en/stable/parameter.html?highlight=0.3#parameters-for-tree-booster
            double learning_rate = 0.3; // TODO: Hard-coding XGBoost Library default value. Change this to a parameter if modified.
            for (InputIt it = begin; it != end; ++it) {
                if (counts.find(*it) != counts.end()) {
                    ++counts[*it];
                }
                else {
                    counts[*it] = 1;
                }
            }
            return std::max_element(counts.begin(), counts.end(),[] (const std::pair<T, int>& pair1, const std::pair<T, int>& pair2) {
                return pair1.second < pair2.second;})->first;
        }

        template<class InputIt, class T = typename std::iterator_traits<InputIt>::value_type> T compute_result(InputIt begin, InputIt end) {
            switch(modelType) {
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

        pdb::Handle<pdb::Vector<float>> predict(Handle<FFMatrixBlock>& in) {
            std::vector<std::vector<decisiontree::Node>> vectForest;
            for(int j = 0; j < numTree; j++){
                pdb::Vector<pdb::Handle<decisiontree::Node>> tree = forest[j];
                // set a new vector to store the whole tree
                std::vector<decisiontree::Node> vectNode;
                for(int i = 0; i < tree.size(); i++){
                    pdb::Handle<decisiontree::Node> thisNodePtr = tree[i];
                    decisiontree::Node thisNode = decisiontree::Node(thisNodePtr->nodeID,thisNodePtr->indexID,thisNodePtr->isLeaf,thisNodePtr->leftChild,thisNodePtr->rightChild,thisNodePtr->returnClass);
                    vectNode.push_back(thisNode);
                }
                vectForest.push_back(vectNode);
            }

            // get the input features matrix information
            uint32_t inNumRow = in->getRowNums();
            uint32_t inNumCol = in->getColNums();

            float *inData = in->getValue().rawData->c_ptr();

            // set the output matrix
            pdb::Handle<pdb::Vector<float>> resultMatrix = pdb::makeObject<pdb::Vector<float>>();
            std::vector<float> thisResultMatrix(numTree);

            // set the node of the tree
            decisiontree::Node * treeNode = nullptr;

            float inputValue;

            for (int i = 0; i < inNumRow; i++){
                for(int j = 0; j < numTree; j++){
                    //std::sort(vectNode.begin(), vectNode.end(), compareByNodeID);
                    // inference
                    // pass the root node of the tree
                    treeNode = & vectForest[j].at(0);
                    while(treeNode->isLeaf == false){
                        inputValue = inData[i*inNumCol+treeNode->indexID];
                        if(inputValue <= treeNode->returnClass){
                            treeNode = treeNode + (treeNode->leftChild);
                        }else{
                            treeNode = treeNode + (treeNode->rightChild);
                        }
                    }
                    thisResultMatrix[j] = treeNode->returnClass;
                }
                float voteResult = compute_result(thisResultMatrix.begin(), thisResultMatrix.end());
                resultMatrix->push_back(voteResult);
            }
            return resultMatrix;
        }
    };
}

#endif //NETSDB_FOREST_H
