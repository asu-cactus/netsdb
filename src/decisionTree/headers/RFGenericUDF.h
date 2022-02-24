#ifndef RF_GENERIC_UDF_H
#define RF_GENERIC_UDF_H

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
#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "Tree.h"
#include "TreeNode.h"
#include "RandomForest.h"

using namespace pdb;

namespace decisiontree{

  class RFGenericUDF: public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  
  public:

    ENABLE_DEEP_COPY

    pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> forest;

    static bool compareByNodeID(const decisiontree::Node &a, const decisiontree::Node &b){
      return a.nodeID < b.nodeID;
    }

    RFGenericUDF() {}

    ~RFGenericUDF() {}

    RFGenericUDF(pdb::Vector<std::string> treePathIn, std::string typeIn) {

      // the # of trees of the input
      int numTrees = treePathIn.size();
      // "C" represents classification and "R" represents regression
      std::string type = typeIn;

      for(int n = 0; n < numTrees; n++){
        std::string inputFileName = std::string(treePathIn[n]);
        std::ifstream infile;
        infile.open(inputFileName.data()); // link the ifstream with the file name
        assert(infile.is_open()); // stop the program if failed

        std:: string line;
        std::vector<std::string> relationships;
        std::vector<std::string> innerNodes;
        std::vector<std::string> leafNodes;
        string::size_type position;

        bool leftLine = true;
        bool rightLine = true;

        while(getline(infile,line)){
          if(line == "digraph Tree {" || line == "node [shape=box] ;" || line == "}"){
            continue;
          }else{
            position = line.find("->");
            if (position != string::npos){
              if(line.find("0 -> 1") != string::npos && line.find("True") != string::npos){
                rightLine = false;
              } else{
                leftLine = true;
              }
              relationships.push_back(line);
            }
            else{
              if(line.find("X") != string::npos){
                innerNodes.push_back(line);
              } else{
                leafNodes.push_back(line);
              }
            }
          }
        }

        infile.close();

        int findStartPosition;
        int findMidPosition;
        int findEndPosition;
        pdb::Vector<pdb::Handle<decisiontree::Node>> tree;

        //std::cout << "print the innerNodes" << std::endl;
        for (int i = 0; i < innerNodes.size(); i++) {
          string thisLine = innerNodes[i];
          int nodeID;
          int indexID;
          double returnClass;
          //std::cout << innerNodes[i] << std::endl;

          if((findEndPosition = thisLine.find_first_of("[label=")) != string::npos){
            nodeID = std::stoi(thisLine.substr(0, findEndPosition-1));
            //std::cout << nodeID << std::endl;
          }
          if((findStartPosition = thisLine.find("X")) != string::npos && (findEndPosition = thisLine.find_first_of("]")) != string::npos){
            indexID = std::stoi(thisLine.substr(findStartPosition+2, findEndPosition-(findStartPosition+2)));
            //std::cout << indexID << std::endl;
          }
          if((findStartPosition = thisLine.find("<=")) != string::npos && (findEndPosition = thisLine.find_first_of("ngini")) != string::npos){
            returnClass = std::stod(thisLine.substr(findStartPosition+3, findEndPosition-1-(findStartPosition+3)));
            //std::cout << returnClass << std::endl;
          }
          tree.push_back(pdb::makeObject<decisiontree::Node>(nodeID, indexID, false, -1, -1, returnClass));
        }
        //std::cout << "print the leafNodes" << std::endl;

        for (int i = 0; i < leafNodes.size(); i++) {
          string thisLine = leafNodes[i];
          int nodeID;
          double returnClass = -1.0;
          //std::cout << leafNodes[i] << std::endl;
          if((findEndPosition = thisLine.find_first_of("[label=\"gini")) != string::npos){
            nodeID = std::stoi(thisLine.substr(0, findEndPosition-1));
            //std::cout << nodeID << std::endl;
          }
          if(type == "C"){
            if((findStartPosition = thisLine.find("nvalue = "))!= string::npos && (findMidPosition = thisLine.find_first_of(",")) != string::npos && (findEndPosition = thisLine.find_first_of("]\"] ;")) != string::npos){
              double firstValue = std::stod(thisLine.substr(findStartPosition+10, findMidPosition-(findStartPosition+10)));
              double secondValue = std::stod(thisLine.substr(findMidPosition+2, findEndPosition-(findMidPosition+2)));
              //std::cout << firstValue << " " << secondValue << std::endl;
              if(firstValue >= secondValue){
                returnClass = 1.0;
              }else{
                returnClass = 2.0;
              }
            }
          } else if(type == "R"){
            if((findStartPosition = thisLine.find("nvalue = "))!= string::npos && (findEndPosition = thisLine.find_last_of("\"] ;")) != string::npos){
              returnClass = std::stod(thisLine.substr(findStartPosition+9, findEndPosition-(findStartPosition+9)));
              //std::cout << returnClass << std::endl;
            }
          } else{
            std::cout << "category error" << std::endl;
            exit(1);
          }
          tree.push_back(pdb::makeObject<decisiontree::Node>(nodeID, -1, true, -1, -1, returnClass));
        }
        //std::cout << "number of tree nodes: " << tree.size() << std::endl;
        //std::cout << "print the relationships" << std::endl;
        for (int i = 0; i < relationships.size(); i++) {
          //std::cout << relationships[i] << std::endl;
          int nodeID;
          int thisChild;
          std::string thisLine = relationships[i];
          if((findMidPosition = thisLine.find_first_of("->")) != string::npos){
            nodeID = std::stoi(thisLine.substr(0, findMidPosition-1));
            //std::cout << nodeID << std::endl;
          }
          if(nodeID == 0){
            if((findEndPosition = thisLine.find_first_of(" [")) != string::npos){
              thisChild = std::stoi(thisLine.substr(findMidPosition+3, findEndPosition-1-(findMidPosition+3)));
              //std::cout << thisChild << std::endl;
            }
          } else{
            if((findEndPosition = thisLine.find_first_of(" ;")) != string::npos){
              thisChild = std::stoi(thisLine.substr(findMidPosition+3, findEndPosition-1-(findMidPosition+3)));
              //std::cout << thisChild << std::endl;
            }
          }

          for(int i = 0; i < tree.size(); i++){
            if(tree[i]->nodeID == nodeID){
              if(tree[i]->leftChild == -1){
                tree[i]->leftChild = thisChild;
              }else{
                tree[i]->rightChild = thisChild;
              }
            }
          }
        }

        forest.push_back(tree);
      }

      std::cout << "numbers of trees in the forest: " << forest.size() << std::endl;
      std::cout << "numbers of nodes in each tree: " << std::endl;
      for(int i = 0; i < forest.size(); i++){
        std::cout << "numbers of nodes in tree " << i << " is " << forest[i].size() << std::endl;
      }
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
    
    Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<FFMatrixBlock> &checkMe) { return true; });
    }

    Lambda<Handle<FFMatrixBlock>>
    getProjection(Handle<FFMatrixBlock> in) override {
        return makeLambda(in, [this](Handle<FFMatrixBlock> &in) {
            // load the metadata
            uint32_t inNumRow = in->getRowNums();
            uint32_t inNumCol = in->getColNums();
            uint32_t inBlockRowIndex = in->getBlockRowIndex();
            uint32_t inBlockColIndex = in->getBlockColIndex();

            // testing purpose
            std::cout << "Finish load the metadata" << std::endl;
            std::cout << inNumRow << "," << inNumCol << std::endl;
            std::cout << inBlockRowIndex << "," << inBlockColIndex << std::endl;

            double *inData = in->getValue().rawData->c_ptr();

            // set the output matrix
            pdb::Handle<pdb::Vector<double>> resultMatrix = pdb::makeObject<pdb::Vector<double>>();
            std::vector<double> thisResultMatrix;
            
            // set the node of the tree
            decisiontree::Node * treeNode = nullptr;

            double inputValue;

            for (int i = 0; i < inNumRow; i++){
              for(int j = 0; j < forest.size(); j++){
                pdb::Vector<pdb::Handle<decisiontree::Node>> tree = forest[j];
                // set a new vetor to store the whole tree
                std::vector<decisiontree::Node> vectNode;
                for(int i = 0; i < tree.size(); i++){
                  pdb::Handle<decisiontree::Node> thisNodePtr = tree[i];
                  decisiontree::Node thisNode = decisiontree::Node(thisNodePtr->nodeID,thisNodePtr->indexID,thisNodePtr->isLeaf,thisNodePtr->leftChild,thisNodePtr->rightChild,thisNodePtr->returnClass);
                  vectNode.push_back(thisNode);
                }

                std::sort(vectNode.begin(), vectNode.end(), compareByNodeID);

                // inference
                // pass the root node of the tree
                treeNode = & vectNode.at(0);
                while(treeNode->isLeaf == false){
                  inputValue = inData[i*inNumCol+treeNode->indexID];
                  if(inputValue <= treeNode->returnClass){
                    * treeNode = * (treeNode + (treeNode->leftChild));
                  }else{
                    * treeNode = * (treeNode + (treeNode->rightChild));
                  }
                }
                thisResultMatrix.push_back(treeNode->returnClass);
              }
              double voteResult = most_common(thisResultMatrix.begin(), thisResultMatrix.end());
              resultMatrix->push_back(voteResult);
              thisResultMatrix.clear();
            }
            pdb::Handle<FFMatrixBlock> resultMatrixBlock = pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, inNumRow, 1, resultMatrix);
            return resultMatrixBlock;
          });
      }
    };
  }

#endif