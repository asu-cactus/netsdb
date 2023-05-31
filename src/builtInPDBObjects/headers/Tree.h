
#ifndef NETSDB_TREE_H
#define NETSDB_TREE_H

#define MAX_NUM_NODES_PER_TREE 512

#include "DataTypes.h"
#include "PDBClient.h"
#include "StorageClient.h"
#include "TensorBlock2D.h"
#include "TreeResult.h"
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

// PRELOAD %Tree%

using std::filesystem::directory_iterator;

namespace pdb {

class Tree : public Object {
  private:
    inline static std::array<std::string, 15> parseLGBMOneCsvLine(const std::string &line) {
        std::array<std::string, 15> fields;
        int findEndPosition = 0;
        int findStartPosition = 0;
        for (int i = 0; i < 14; ++i) {
            findEndPosition = line.find(",", findStartPosition);
            fields[i] = line.substr(findStartPosition, findEndPosition - findStartPosition);
            findStartPosition = findEndPosition + 1;
        }
        fields[14] = line.substr(findStartPosition);
        return fields;
    }

  public:
    Node tree[MAX_NUM_NODES_PER_TREE];

    Tree() {}

    int treeId;

    ModelType modelType;

    Tree(int treeId, std::string_view treePath, ModelType modelType)
        : treeId{treeId}, modelType{modelType} {
        this->constructTreeFromPath(treePath, modelType, this->tree);
    }

    void setUpAndCopyFrom(void *target, void *source) const override {
        new (target) Tree();
        Tree &fromMe = *((Tree *)source);
        Tree &toMe = *((Tree *)target);
        for (int i = 0; i < MAX_NUM_NODES_PER_TREE; i++) {
            toMe.tree[i].indexID = fromMe.tree[i].indexID;
            toMe.tree[i].isLeaf = fromMe.tree[i].isLeaf;
            toMe.tree[i].leftChild = fromMe.tree[i].leftChild;
            toMe.tree[i].rightChild = fromMe.tree[i].rightChild;
            toMe.tree[i].threshold = fromMe.tree[i].threshold;
        }
        toMe.treeId = fromMe.treeId;
        toMe.modelType = fromMe.modelType;
    }

    void deleteObject(void *deleteMe) override {
        deleter(deleteMe, this);
    }

    size_t getSize(void *forMe) override {
        return sizeof(Tree);
    }

    static void processInnerNodes(std::vector<std::string> &innerNodes, ModelType modelType, Node *tree) {

        int findStartPosition;
        int findMidPosition;
        int findEndPosition;

        for (int i = 0; i < innerNodes.size(); ++i) {
            // Construct Inner Nodes
            const string &currentLine = innerNodes[i];
            int nodeID;
            int indexID;
            float threshold;

            // to get nodeID
            if (modelType == ModelType::RandomForest) {
                if ((findEndPosition = currentLine.find("[label")) != string::npos) {
                    // std::cout << "nodeID==" << currentLine.substr(0, findEndPosition - 1) << '\n';
                    nodeID = std::stoi(currentLine.substr(0, findEndPosition - 1));
                } else {
                    std::cout << "Error in extracting inner node nodeID\n";
                    std::cout << "currentLine: " << currentLine << '\n';
                    exit(1);
                }

                if ((findStartPosition = currentLine.find("X[")) != string::npos && (findEndPosition = currentLine.find("] <=")) != string::npos) {
                    // std::cout << "indexID==" << currentLine.substr(findStartPosition + 2, findEndPosition - findStartPosition - 2) << '\n';
                    indexID = std::stoi(currentLine.substr(findStartPosition + 2, findEndPosition - findStartPosition - 2));
                } else {
                    std::cout << "Error in extracting inner node indexID\n";
                    std::cout << "currentLine: " << currentLine << '\n';
                    exit(1);
                }

                findStartPosition = currentLine.find("<=");
                if (findStartPosition != string::npos && (findEndPosition = currentLine.find("\\ngini")) != string::npos) {
                    // For Classifiers
                    // std::cout << "threshold==" << currentLine.substr(findStartPosition + 3, findEndPosition - findStartPosition - 3) << '\n';
                    threshold = std::stod(currentLine.substr(findStartPosition + 3, findEndPosition - findStartPosition - 3));
                } else if (findStartPosition != string::npos && (findEndPosition = currentLine.find("\\nsqu")) != string::npos) {
                    // For Regressors
                    // std::cout << "threshold==" << currentLine.substr(findStartPosition + 3, findEndPosition - findStartPosition - 3) << '\n';
                    threshold = std::stod(currentLine.substr(findStartPosition + 3, findEndPosition - findStartPosition - 3));
                } else {
                    std::cout << "Error in extracting inner node threshold\n";
                    std::cout << "currentLine: " << currentLine << '\n';
                    exit(1);
                }

                tree[nodeID].isMissTrackLeft = false; // Doesn't matter to RandomForest
            } else if (modelType == ModelType::XGBoost) {
                if ((findEndPosition = currentLine.find("[ label")) != string::npos) {
                    // std::cout << "nodeID==" << currentLine.substr(4, findEndPosition - 1 - 4) << '\n';
                    nodeID = std::stoi(currentLine.substr(4, findEndPosition - 1 - 4));
                } else {
                    std::cout << "Error in extracting inner node nodeID\n";
                    exit(1);
                }

                if ((findStartPosition = currentLine.find("f")) != string::npos && (findEndPosition = currentLine.find("<")) != string::npos) {
                    // std::cout << "indexID==" << currentLine.substr(findStartPosition + 1, findEndPosition - findStartPosition - 1) << '\n';
                    indexID = std::stoi(currentLine.substr(findStartPosition + 1, findEndPosition - findStartPosition - 1));
                } else {
                    std::cout << "Error in extracting inner node indexID\n";
                    exit(1);
                }

                if ((findStartPosition = currentLine.find("<")) != string::npos && (findEndPosition = currentLine.find("\" ]")) != string::npos) {
                    // std::cout << "threshold==" << currentLine.substr(findStartPosition + 1, findEndPosition - findStartPosition - 1) << '\n';
                    threshold = std::stod(currentLine.substr(findStartPosition + 1, findEndPosition - findStartPosition - 1));
                } else {
                    std::cout << "Error in extracting inner node threshold\n";
                    exit(1);
                }
                tree[nodeID].isMissTrackLeft = false; // XGBoost default is no/right
            } else {                                  // modelType == ModelType::LightGBM
#if 1
                auto fields = parseLGBMOneCsvLine(currentLine);
                nodeID = std::stoi(fields[2].substr(fields[2].find("S") + 1));
                indexID = std::stoi(fields[6].substr(7));
                threshold = std::stod(fields[8]);
#else

                if (((findStartPosition = currentLine.find_first_of("split")) != string::npos) && ((findEndPosition = currentLine.find_first_of("[label=<<B>")) != string::npos)) {
                    nodeID = std::stoi(currentLine.substr(findStartPosition + 5, findEndPosition - 1 - findStartPosition - 5));
                }
                if ((findStartPosition = currentLine.find("Column_")) != string::npos) {
                    if ((findEndPosition = currentLine.find("</B> &#8804;")) != string::npos) {
                        indexID = std::stoi(currentLine.substr(findStartPosition + 7, findEndPosition - findStartPosition - 7));
                    }
                } else {

                    if (((findStartPosition = currentLine.find("[label=<<B>")) != string::npos) && ((findEndPosition = currentLine.find("</B> &#8804;")) != string::npos)) {
                        std::cout << findStartPosition << std::endl;
                        std::cout << findEndPosition << std::endl;
                        std::cout << currentLine.substr(findStartPosition + 11, findEndPosition - findStartPosition - 11) << std::endl;
                        indexID = std::stoi(currentLine.substr(findStartPosition + 11, findEndPosition - findStartPosition - 11));
                    }
                }
                if ((findStartPosition = currentLine.find("&#8804;<B>")) != string::npos && (findEndPosition = currentLine.find_first_of("</B>> fillcolor=")) != string::npos) {
                    leafValue = std::stod(currentLine.substr(findStartPosition + 10, findEndPosition - findStartPosition - 10));
                }

#endif
                tree[nodeID].isMissTrackLeft = true; // LigthGBM default is left
            }
            tree[nodeID].indexID = indexID;
            tree[nodeID].isLeaf = false;
            tree[nodeID].leftChild = -1;
            tree[nodeID].rightChild = -1;
            tree[nodeID].threshold = threshold;
        }
    }

    static void processLeafNodes(std::vector<std::string> &leafNodes, ModelType modelType, Node *tree) {
        int findStartPosition;
        int findMidPosition;
        int findEndPosition;

        for (int i = 0; i < leafNodes.size(); ++i) {
            // Construct Leaf Nodes
            const string &currentLine = leafNodes[i];

            int nodeID;
            float leafValue = -1.0f;
            if (modelType == ModelType::RandomForest) {
                if ((findEndPosition = currentLine.find("[label=\"gini")) != string::npos ||
                    (findEndPosition = currentLine.find("[label=\"squ")) != string::npos) {
                    // std::cout << "nodeID in leaf==" << currentLine.substr(0, findEndPosition - 1) << '\n';
                    nodeID = std::stoi(currentLine.substr(0, findEndPosition - 1));
                } else {
                    std::cout << "Error in extracting leaf node nodeID\n";
                    exit(1);
                }
#if 1
                if ((findStartPosition = currentLine.find("value = [")) != string::npos && (findEndPosition = currentLine.find("]\\nclass")) != string::npos) {
                    // For classifiers
                    // std::cout << "leafValue from pairs in leaf==" << currentLine.substr(findStartPosition + 9, findEndPosition - findStartPosition - 9) << '\n';
                    const std::string pairs = currentLine.substr(findStartPosition + 9, findEndPosition - findStartPosition - 9);
                    findMidPosition = pairs.find(", ");
                    const int classZeroCounts = std::stoi(pairs.substr(0, findMidPosition));
                    const int classOneCounts = std::stoi(pairs.substr(findMidPosition + 2));
                    leafValue = classOneCounts / static_cast<float>(classZeroCounts + classOneCounts);
                } else if ((findStartPosition = currentLine.find("value = ")) != string::npos || (findEndPosition = currentLine.find("\"]")) != string::npos) {
                    // For Regressors
                    // std::cout << "leafValue from pairs in leaf==" << currentLine.substr(findStartPosition + 8, findEndPosition - findStartPosition - 8) << '\n';
                    leafValue = std::stof(currentLine.substr(findStartPosition + 8, findEndPosition - findStartPosition - 8));
                } else {
                    std::cout << "Error in extracting leaf node leafValue\n";
                    exit(1);
                }

#else
                // Output Class of always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                if ((findStartPosition = currentLine.find("y[")) != string::npos && (findEndPosition = currentLine.find("]\"]")) != string::npos) {
                    leafValue = std::stod(currentLine.substr(findStartPosition + 2, findEndPosition - findStartPosition - 2));
                }
#endif
            } else if (modelType == ModelType::XGBoost) {
                if ((findEndPosition = currentLine.find("[")) != string::npos) {
                    // std::cout << "nodeID in leaf==" << currentLine.substr(4, findEndPosition - 1 - 4) << '\n';
                    nodeID = std::stoi(currentLine.substr(4, findEndPosition - 1 - 4));
                } else {
                    std::cout << "Error in extracting leaf node nodeID\n";
                    exit(1);
                }

                // Output Class of XGBoost always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                if ((findStartPosition = currentLine.find("leaf=")) != string::npos && (findEndPosition = currentLine.find("\" ]")) != string::npos) {
                    // std::cout << "leafValue in leaf==" << currentLine.substr(findStartPosition + 5, findEndPosition - 3 - findStartPosition - 5) << '\n';
                    leafValue = std::stod(currentLine.substr(findStartPosition + 5, findEndPosition - 3 - findStartPosition - 5));
                } else {
                    std::cout << "Error in extracting leaf node leafValue\n";
                    exit(1);
                }

            } else { // modelType == ModelType::LightGBM
#if 1
                auto fields = parseLGBMOneCsvLine(currentLine);
                nodeID = std::stoi(fields[2].substr(fields[2].find("L") + 1));
                leafValue = std::stod(fields[12]);
#else
                if (((findStartPosition = currentLine.find_first_of("leaf")) != string::npos) && ((findEndPosition = currentLine.find_first_of("[")) != string::npos)) {
                    nodeID = std::stoi(currentLine.substr(findStartPosition + 4, findEndPosition - 1 - findStartPosition - 4));
                }
                // Output Class of XGBoost always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                if ((findStartPosition = currentLine.find(": <B>")) != string::npos && (findEndPosition = currentLine.find("</B>>]")) != string::npos) {
                    leafValue = std::stod(currentLine.substr(findStartPosition + 5, findEndPosition - findStartPosition - 5));
                }
#endif
                nodeID = nodeID + MAX_NUM_NODES_PER_TREE / 2;
            }

            tree[nodeID].indexID = -1;
            tree[nodeID].isLeaf = true;
            tree[nodeID].leftChild = -1;
            tree[nodeID].rightChild = -1;
            tree[nodeID].leafValue = leafValue;
            tree[nodeID].isMissTrackLeft = true; // Doesn't matter to leave nodes
        }
    }

    static void processRelationships(std::vector<std::string> &relationships, ModelType modelType, Node *tree) {
        int findStartPosition;
        int findMidPosition;
        int findEndPosition;

        for (int i = 0; i < relationships.size(); ++i) {
            // Construct Directed Edges between Nodes
            const std::string &currentLine = relationships[i];
            int parentNodeID;
            int childNodeID;

            if (modelType == ModelType::RandomForest) {
                if ((findMidPosition = currentLine.find("->")) != std::string::npos) {
                    // std::cout << "parentNodeID==" << currentLine.substr(0, findMidPosition - 1) << '\n';
                    parentNodeID = std::stoi(currentLine.substr(0, findMidPosition - 1));
                } else {
                    std::cout << "Error in extracting parentNodeID\n";
                    exit(1);
                }

                std::string endPositionStr = (parentNodeID == 0) ? "[label" : ";";
                if ((findEndPosition = currentLine.find(endPositionStr)) != std::string::npos) {
                    // std::cout << "childNodeID==" << currentLine.substr(findMidPosition + 3, findEndPosition - 1 - findMidPosition - 3) << '\n';
                    childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition - 1 - findMidPosition - 3));
                } else {
                    std::cout << "Error in extracting childNodeID\n";
                    exit(1);
                }

            } else if (modelType == ModelType::XGBoost) {
                if ((findMidPosition = currentLine.find("->")) != std::string::npos) {
                    // std::cout << "parentNodeID==" << currentLine.substr(4, findMidPosition - 1 - 4) << '\n';
                    parentNodeID = std::stoi(currentLine.substr(4, findMidPosition - 1 - 4));
                } else {
                    std::cout << "Error in extracting parentNodeID\n";
                    exit(1);
                }

                if ((findEndPosition = currentLine.find("[")) != std::string::npos) {
                    // std::cout << "childNodeID==" << currentLine.substr(findMidPosition + 3, findEndPosition - 1 - findMidPosition - 3) << '\n';
                    childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition - 1 - findMidPosition - 3));
                } else {
                    std::cout << "Error in extracting childNodeID\n";
                    exit(1);
                }

                if (currentLine.find("yes, missing") != std::string::npos) {
                    tree[parentNodeID].isMissTrackLeft = true; // in processInnerNodes(), default value is set to no/right
                }
            } else { // modelType == ModelType::LightGBM
#if 1
                auto fields = parseLGBMOneCsvLine(currentLine);
                parentNodeID = std::stoi(fields[2].substr(fields[2].find("S") + 1));
                const std::string leftChildString = fields[3];
                const std::string rightChildString = fields[4];

                if (!leftChildString.empty()) {
                    tree[parentNodeID].leftChild = std::stoi(leftChildString.substr(leftChildString.find("-") + 2));
                    bool isLeftChildLeaf = (leftChildString.find("L") != std::string::npos);
                    if (isLeftChildLeaf) {
                        tree[parentNodeID].leftChild += MAX_NUM_NODES_PER_TREE / 2;
                    }
                }

                if (!rightChildString.empty()) {
                    tree[parentNodeID].rightChild = std::stoi(rightChildString.substr(rightChildString.find("-") + 2));
                    bool isRighChildLeaf = (rightChildString.find("L") != std::string::npos);
                    if (isRighChildLeaf) {
                        tree[parentNodeID].rightChild += MAX_NUM_NODES_PER_TREE / 2;
                    }
                }
                tree[parentNodeID].isMissTrackLeft = (fields[10].compare("left") == 0) ? true : false;
#else

                if (((findStartPosition = currentLine.find_first_of("split")) != std::string::npos) && ((findMidPosition = currentLine.find_first_of("->")) != std::string::npos)) {

                    parentNodeID = std::stoi(currentLine.substr(findStartPosition + 5, findMidPosition - 1 - findStartPosition - 5));
                }
                if ((findEndPosition = currentLine.find("[")) != std::string::npos) {
                    if (currentLine.find("leaf") != std::string::npos) {
                        childNodeID = std::stoi(currentLine.substr(findMidPosition + 7, findEndPosition - 1 - findMidPosition - 7));
                        childNodeID = childNodeID + MAX_NUM_NODES_PER_TREE / 2;
                    } else {
                        childNodeID = std::stoi(currentLine.substr(findMidPosition + 8, findEndPosition - 1 - findMidPosition - 8));
                    }
                }
#endif
            }

            if (modelType != ModelType::LightGBM) {
                // This if statement should be deleted if the older version of parsing LightGBM is used.
                if (tree[parentNodeID].leftChild == -1) {
                    tree[parentNodeID].leftChild = childNodeID;
                } else if (tree[parentNodeID].rightChild == -1) {
                    tree[parentNodeID].rightChild = childNodeID;
                } else {
                    std::cout << "Error in parsing trees: children nodes were updated again: " << parentNodeID << "->" << childNodeID << std::endl;
                }
            }
        }
    }

    static void constructTreeFromPath(std::string_view treePathIn, ModelType modelType, Node *tree) {
        std::vector<std::string> relationships;
        std::vector<std::string> innerNodes;
        std::vector<std::string> leafNodes;
        constructTreeFromPathHelper(treePathIn, modelType, relationships, innerNodes, leafNodes);
        processInnerNodes(innerNodes, modelType, tree);
        processLeafNodes(leafNodes, modelType, tree);
        processRelationships(relationships, modelType, tree);
    }

    static void constructTreeFromPathHelper(
        std::string_view treePathIn,
        ModelType modelType,
        std::vector<std::string> &relationships,
        std::vector<std::string> &innerNodes,
        std::vector<std::string> &leafNodes) {

        std::ifstream inputFile;
        inputFile.open(treePathIn.data());
        assert(inputFile.is_open());

        std::string line;
        switch (modelType) {
        case ModelType::RandomForest:
            while (getline(inputFile, line)) {
                if ((line.size() == 0) || (line.find("digraph Tree {") != std::string::npos) || (line.find("node [shape=box") != std::string::npos) ||
                    (line.find("edge [fontname=") != std::string::npos) || (line.find("}") != std::string::npos)) {
                } else {

                    if (line.find("->") != string::npos) {
                        relationships.push_back(line);
                    } else if ((line.find("[label=\"gini") != string::npos) || (line.find("[label=\"squ") != string::npos)) {
                        leafNodes.push_back(line);
                    } else {
                        innerNodes.push_back(line);
                    }
                }
            }
            break;
        case ModelType::XGBoost:
            while (getline(inputFile, line)) {
                if ((line.size() == 0) || (line.find("graph") != std::string::npos) || (line.find("}") != std::string::npos)) {
                } else {

                    if (line.find("->") != string::npos) {
                        relationships.push_back(line);
                    } else if (line.find("leaf") != string::npos) {
                        leafNodes.push_back(line);
                    } else {
                        innerNodes.push_back(line);
                    }
                }
            }
            break;
        case ModelType::LightGBM:
            getline(inputFile, line); // Skip header
            while (getline(inputFile, line)) {
                if (line.find(",,,") != std::string::npos) { // Leaf nodes have many empty entries
                    leafNodes.push_back((line));
                } else { // Lines for inner nodes also contain full information of relations
                    innerNodes.push_back(line);
                    relationships.push_back(line);
                }
            }
            break;
        }
        inputFile.close();
    }

    pdb::Handle<TreeResult> predictWithMissingValues(Handle<TensorBlock2D<float>> &in) {
        return predict<true>(in);
    }

    pdb::Handle<TreeResult> predictWithoutMissingValues(Handle<TensorBlock2D<float>> &in) {
        return predict<false>(in);
    }

    template <bool hasMissing>
    inline pdb::Handle<TreeResult> predict(Handle<TensorBlock2D<float>> &in) const { // TODO: Change all Double References to Float
        // get the input features matrix information
        uint32_t rowIndex = in->getBlockRowIndex();
        int numRows = in->getRowNums();
        int numCols = in->getColNums();
        float *inData = in->getValue().rawData->c_ptr();

        // set the output matrix
        pdb::Handle<TreeResult> resultMatrix = pdb::makeObject<TreeResult>(treeId, rowIndex, numRows, modelType);

        float *outData = resultMatrix->data->c_ptr();

        constexpr int batchSize{4000};
        const int remainderSize{numRows % batchSize};
        const bool hasRemainder{remainderSize != 0};
        const int iterations = numRows / batchSize + hasRemainder;
        for (int featureStartIndex{0}, base{0}, rowNumBase{0}, i{0}; i < iterations; i++) {
            base = i * batchSize * numCols;
            rowNumBase = i * batchSize;
            const int blockSize = (hasRemainder && i + 1 == iterations) ? remainderSize : batchSize;
            for (int j = 0; j < blockSize; j++) {
                featureStartIndex = base + j * numCols;
                int curIndex = 0;
                while (tree[curIndex].isLeaf == false) {
                    const double featureValue = inData[featureStartIndex + tree[curIndex].indexID];
                    if (hasMissing && std::isnan(featureValue)) {
                        curIndex = tree[curIndex].isMissTrackLeft
                                       ? tree[curIndex].leftChild
                                       : tree[curIndex].rightChild;
                    } else {
                        curIndex = featureValue < tree[curIndex].threshold
                                       ? tree[curIndex].leftChild
                                       : tree[curIndex].rightChild;
                    }
                }
                outData[rowNumBase + j] = (float)(tree[curIndex].leafValue);
            }
        }
        return resultMatrix;
    }
}; // Tree
} // namespace pdb

#endif // NETSDB_TREE_H
