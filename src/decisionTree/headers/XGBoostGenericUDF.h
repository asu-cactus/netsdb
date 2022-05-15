#ifndef XGBOOST_GENERIC_UDF_H
#define XGBOOST_GENERIC_UDF_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
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
#include <algorithm>
#include <map>
#include <set>
#include <cstring>
#include <exception>

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "Tree.h"
#include "TreeNode.h"
#include "Forest.h"

using namespace pdb;

namespace decisiontree {
    class XGBoostGenericUDF : public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
    private:
        void constructForestForXGBoost(pdb::Vector<std::string> treePathIn, bool isClassification) {
            for(int n=0; n<num_trees;++n) {
                std::string inputFileName = std::string(treePathIn[n]);
                std::ifstream inputFile;
                inputFile.open(inputFileName.data());
                assert(inputFile.is_open());

                std::string line;
                std::vector<std::string> relationships;
                std::vector<std::string> innerNodes;
                std::vector<std::string> leafNodes;
                string::size_type position;

//                bool leftLine = true;
//                bool rightLine = true;

                while(getline(inputFile, line)) {
                    if(line == "digraph Tree {" || line == "node [shape=box] ;" || line == "}"){
                        continue;
                    }
                    else {
                        position = line.find("->");
                        if (position != string::npos) {
//                            if(line.find("0 -> 1") != string::npos && line.find("True") != string::npos) { // Identify L/R Subtrees
//                                rightLine = false;
//                            }
                            relationships.push_back(line);
                        }
                        else { // Find Leaf/Inner Node
                            if(line.find("leaf") != string::npos) {
                                leafNodes.push_back(line);
                            }
                            else {
                                innerNodes.push_back(line);
                            }
                        }
                    }
                }

                inputFile.close();

                int findStartPosition;
                int findMidPosition;
                int findEndPosition;
                pdb::Vector<pdb::Handle<decisiontree::Node>> tree;

                for(int i=0;i<innerNodes.size();++i) { // Construct Inner Nodes
                    string currentLine = innerNodes[i];
                    int nodeID;
                    int indexID;
                    float returnClass;

                    if((findEndPosition = currentLine.find_first_of("label")) != string::npos) {
                        nodeID = std::stoi(currentLine.substr(0,findEndPosition-2));
                    }
                    if((findStartPosition = currentLine.find("=\"[f")) != string::npos && (findEndPosition = currentLine.find("<")) != string::npos) { // Verified there is no > character for Inner node
                        indexID = std::stoi(currentLine.substr(findStartPosition+4, findEndPosition));
                    }
                    if((findStartPosition = currentLine.find("<")) != string::npos && (findEndPosition = currentLine.find_first_of("]")) != string::npos) { // Verified there is no > character for Inner node
                        returnClass = std::stof(currentLine.substr(findStartPosition+1, findEndPosition));
                    }
                    tree.push_back(pdb::makeObject<decisiontree::Node>(nodeID, indexID, false, -1, -1, returnClass));
                }

                for(int i=0;i<leafNodes.size();++i) { // Construct Leaf Nodes
                    string currentLine = leafNodes[i];
                    int nodeID;
                    float returnClass = -1.0f;
                    if((findEndPosition = currentLine.find_first_of("label")) != string::npos) {
                        nodeID = std::stoi(currentLine.substr(0, findEndPosition-2));
                    }
                    // Output Class of XGBoost always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                    if((findStartPosition = currentLine.find("leaf")) != string::npos && (findEndPosition = currentLine.find("]")) != string::npos) {
                        returnClass = std::stof(currentLine.substr(findStartPosition+5, findEndPosition-1));
                    }
                    tree.push_back(pdb::makeObject<decisiontree::Node>(nodeID, -1, true, -1, -1, returnClass));
                }

                for(int i=0;i<relationships.size();++i) { // Construct Directed Edges between Nodes
                    int parentNodeID;
                    int childNodeID;
                    std::string currentLine = relationships[i];
                    if((findMidPosition = currentLine.find_first_of("->")) != std::string::npos) {
                        parentNodeID = std::stoi(currentLine.substr(0, findMidPosition-1));
                    }
                    if(parentNodeID==0) {
                        if((findEndPosition = currentLine.find_first_of(" [")) != std::string::npos) {
                            childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition-1-(findMidPosition+3)));
                        }
                    }
                    else {
                        if((findEndPosition = currentLine.find_first_of(" ;")) != std::string::npos) {
                            childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition-1-(findMidPosition+3)));
                        }
                    }

                    // decisiontree::Node& currParentNode = std::find_if(tree.begin(),tree.end(),[&pNodeID = parentNodeID](const decisiontree::Node& iNode) -> bool { return pNodeID == iNode->nodeID; });
                    // if(currParentNode) {
                    //     if(currParentNode->leftChild == -1) {
                    //         currParentNode->leftChild = childNodeID;
                    //     }
                    //     else {
                    //         currParentNode->rightChild = childNodeID;
                    //     }
                    // }
                    
                    for(int i=0;i<tree.size();++i) {
                        if(tree[i]->nodeID == parentNodeID) {
                            if(tree[i]->leftChild == -1) {
                                tree[i]->leftChild = childNodeID;
                            }
                            else {
                                tree[i]->rightChild = childNodeID;
                            }
                        }
                    }
                    
                }

                forest.push_back(tree);
            }
            // STATS ABOUT THE FOREST
            std::cout << "Number of trees in the forest: " << num_trees << std::endl;
            std::cout << "Number of nodes in each tree: " << std::endl;
            for(int i = 0; i < num_trees; i++){
                std::cout << "Number of nodes in tree[" << i << "] is: " << forest[i].size() << std::endl;
            }
        }

    public:
        ENABLE_DEEP_COPY

        pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> forest;
        std::vector<std::vector<decisiontree::Node>> vectorForest;
        int num_trees;
        pdb::Handle<decisiontree::Node> thisNodePtr;

        XGBoostGenericUDF() {}
        ~XGBoostGenericUDF() {}

        XGBoostGenericUDF(pdb::Vector<std::string> treePathIn, bool isClassificationTask) {
            num_trees = treePathIn.size();
            constructForestForXGBoost(treePathIn, isClassificationTask);
        }

        Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
            return makeLambda(checkMe,
                              [](Handle<FFMatrixBlock> &checkMe) { return true; });
        }

        Lambda<Handle<FFMatrixBlock>> getProjection(Handle<FFMatrixBlock> in) override {
            return makeLambda(in, [this](Handle<FFMatrixBlock> &in) {
                for(int j = 0; j < num_trees; j++){
                    pdb::Vector<pdb::Handle<decisiontree::Node>> tree = forest[j];
                    // set a new vector to store the whole tree
                    std::vector<decisiontree::Node> vectorNode;
                    for(int k = 0; k < tree.size(); k++){
                        thisNodePtr = tree[k];
                        decisiontree::Node thisNode = decisiontree::Node(thisNodePtr->nodeID,thisNodePtr->indexID,thisNodePtr->isLeaf,thisNodePtr->leftChild,thisNodePtr->rightChild,thisNodePtr->returnClass);
                        vectorNode.push_back(thisNode);
                    }
                    vectorForest.push_back(vectorNode);
                }

                // load the metadata
                uint32_t inNumRow = in->getRowNums();
                uint32_t inNumCol = in->getColNums();
                uint32_t inBlockRowIndex = in->getBlockRowIndex();
                uint32_t inBlockColIndex = in->getBlockColIndex();

                // float *inData = in->getValue().rawData->c_ptr();

                // set the output matrix
                pdb::Handle<pdb::Vector<float>> resultMatrix = pdb::makeObject<pdb::Vector<float>>();
                std::vector<float> thisResultMatrix(num_trees);

                // set the node of the tree
                // decisiontree::Node * treeNode = nullptr;
                // float inputValue;

                // Instantiate Forest Object
                Forest::ModelType modelType = Forest::ModelType::XGBoost;
                Forest forest = Forest(modelType);

                pdb::Handle<pdb::Vector<float>> resultMatrix = forest.predict(in);

                // for (int i = 0; i < inNumRow; i++){
                //     for(int j = 0; j < num_trees; j++){
                //         treeNode = &vectorForest[j].at(0); // Root Node
                //         while(treeNode->isLeaf == false){
                //             inputValue = inData[i*inNumCol+treeNode->indexID];
                //             if(inputValue <= treeNode->returnClass){
                //                 treeNode = (treeNode + (treeNode->leftChild));
                //             }else{
                //                 treeNode = (treeNode + (treeNode->rightChild));
                //             }
                //         }
                //         thisResultMatrix[j] = treeNode->returnClass;
                //     }
                //     float aggregateResult = decisiontree::Forest::aggregate_decisions(thisResultMatrix.begin(), thisResultMatrix.end());
                //     resultMatrix->push_back(aggregateResult);
                // }

                pdb::Handle<FFMatrixBlock> resultMatrixBlock = pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, inNumRow, 1, resultMatrix);

                return resultMatrixBlock;
            });
        }
    };
}

#endif