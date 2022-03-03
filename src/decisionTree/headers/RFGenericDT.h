#ifndef RF_GENERIC_DT_H
#define RF_GENERIC_DT_H

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

class RFGenericDT: public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  private:
    decisiontree::RandomForest * forestPtr = nullptr;
    int * Infoptr = nullptr;
    int numTrees;
    pdb::String fileName = "";
    std::vector<int> rootNodes;

  public:

    ENABLE_DEEP_COPY

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

    RFGenericDT() {}
    ~RFGenericDT() {}

    RFGenericDT(pdb::String fileNameIn) {
      fileName = fileNameIn;
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
            //std::cout << "Finish load the metadata" << std::endl;
            //std::cout << inNumRow << "," << inNumCol << std::endl;
            //std::cout << inBlockRowIndex << "," << inBlockColIndex << std::endl;

            float *inData = in->getValue().rawData->c_ptr();

            if(this->forestPtr == nullptr){
              ifstream fin(fileName);
              string line;
              while (getline(fin, line)){
                long long result=strtoll(line.c_str(), NULL, 16);
                this->forestPtr = (decisiontree::RandomForest *)result;
              }
            }

            // testing purpose
            //std::cout << "Address of the node pointer: " << Nodeptr << std::endl;
            //std::cout << "Address of the RF information pointer: " << Infoptr << std::endl;

            /*
            for(int i = 0; i < *Infoptr; i++){
              this->rootNodes.push_back(*(Infoptr+i+1));
            }
            this->numTrees = this->rootNodes.size();
            */

            // set the output matrix
            pdb::Handle<pdb::Vector<float>> resultMatrix = pdb::makeObject<pdb::Vector<float>>();
            std::vector<float> thisResultMatrix;

            //pdb::Handle<pdb::Vector<float>> resultMatrix = pdb::makeObject<pdb::Vector<float>>();

            float inputValue;
            //std::vector<float> thisResultMatrix;
            decisiontree::Node * treeNode = nullptr;
            int numTree = forestPtr->numTree;
            pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> thisForest = forestPtr->forest;

            for (int i = 0; i < inNumRow; i++){
              for(int j = 0; j < numTree; j++){
                pdb::Vector<pdb::Handle<decisiontree::Node>> tree = thisForest[j];
                // set a new vetor to store the whole tree
                std::vector<decisiontree::Node> vectNode;
                for(int i = 0; i < tree.size(); i++){
                  pdb::Handle<decisiontree::Node> thisNodePtr = tree[i];
                  decisiontree::Node thisNode = decisiontree::Node(thisNodePtr->nodeID,thisNodePtr->indexID,thisNodePtr->isLeaf,thisNodePtr->leftChild,thisNodePtr->rightChild,thisNodePtr->returnClass);
                  vectNode.push_back(thisNode);
                }

                std::sort(vectNode.begin(), vectNode.end(), decisiontree::RandomForest::compareByNodeID);

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
              float voteResult = most_common(thisResultMatrix.begin(), thisResultMatrix.end());
              resultMatrix->push_back(voteResult);
              thisResultMatrix.clear();
            }
            pdb::Handle<FFMatrixBlock> resultMatrixBlock = pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, inNumRow, 1, resultMatrix);
            return resultMatrixBlock;
        });
    }
};

#endif