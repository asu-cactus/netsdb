//
// Created by venkateshgunda on 5/10/22.
//

#ifndef NETSDB_FOREST_H
#define NETSDB_FOREST_H

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

        Forest() {
            this->modelType = ModelType::RandomForest;
        }

        Forest(ModelType type) {
            this->modelType = type;
        }

        Forest(pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> forestIn, ModelType type=ModelType::RandomForest) {
            this->forest = forestIn;
            this->numTrees = forestIn.size();
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
            numTrees = forestIn.size();
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

        // Decision of an XGBoost Tree (for class-1, not class-0): sigmoid(log(previous_tree_pred(initial_value=0)/(1-previous_tree_pred(initial_value=0))) + learning_rate*(current_tree_prob))
        template<class InputIt, class T = typename std::iterator_traits<InputIt>::value_type> T aggregate_decisions(InputIt begin, InputIt end){
//          std::map<T, int> counts; // TODO: This function code needs to be re-written for XGBoost
            // Default LR Value Source: https://xgboost.readthedocs.io/en/stable/parameter.html?highlight=0.3#parameters-for-tree-booster
            double learning_rate = 0.3; // TODO: Hard-coding XGBoost Library default value. Change this to a parameter if modified.
            double aggregated_decision = 0.0; // Initial Prediction is 0.5, which makes log(odds) value 0. If different, the default value changes.
            double threshold = 0.5;
            for (InputIt it = begin; it != end; ++it) {
                aggregated_decision += (*it); // Should not be multiplied by Learning Rate
            }
            // Reference: https://stats.stackexchange.com/questions/395697/what-is-an-intuitive-interpretation-of-the-leaf-values-in-xgboost-base-learners
            double sigmoid_of_decision = 1/(1+exp(-1.0*aggregated_decision)); // Sigmoid of the aggregated decision is the final output
            return sigmoid_of_decision > threshold ? 2.0 : 1.0; // Delaying Class Assignment. Class Labels as per RF Code is 1.0 and 2.0
        }

        // TODO: Call this function instead of Individual Compute Functions. Instantiate the Class with the Model Type
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

        pdb::Handle<pdb::Vector<double>> predict(Handle<FFMatrixBlock>& in) { // TODO: Change all Double References to Float
            std::vector<std::vector<decisiontree::Node>> vectForest;
            for(int j = 0; j < numTrees; j++){
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

            double *inData = in->getValue().rawData->c_ptr(); // Need to cast from Double to Float

            // set the output matrix
            pdb::Handle<pdb::Vector<double>> resultMatrix = pdb::makeObject<pdb::Vector<double>>();
            std::vector<double> thisResultMatrix(numTrees);

            // set the node of the tree
            decisiontree::Node * treeNode = nullptr;

            float inputValue;

            for (int i = 0; i < inNumRow; i++){
                for(int j = 0; j < numTrees; j++){
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
