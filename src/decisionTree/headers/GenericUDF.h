#ifndef GENERIC_UDF_H
#define GENERIC_UDF_H

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
#include "TreeNode.h"

using namespace pdb;

class GenericUDF: public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  public:

    ENABLE_DEEP_COPY

    GenericUDF() {}

    ~GenericUDF() {}

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

            decisiontree::Node * treeNode = nullptr;
            std::vector<decisiontree::Node> vectNode;
            decisiontree::Node thisNode;
            thisNode= decisiontree::Node(0,7,false,1,2,0.052);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(1,20,false,3,4,16.54);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(2,26,false,5,6,0.225);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(3,13,false,7,8,37.61);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(4,21,false,9,10,20.22);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(5,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(6,23,false,11,12,710.2);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(7,21,false,13,14,33.27);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(8,4,false,15,16,0.091);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(9,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(10,17,false,17,18,0.011);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(11,21,false,19,20,25.95);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(12,1,false,21,22,14.12);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(13,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(14,21,false,23,24,34.14);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(15,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(16,17,false,25,26,0.012);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(17,-1,true,-1,-1,1.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(18,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(19,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(20,9,false,27,28,0.065);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(21,25,false,29,30,0.361);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(22,-1,true,-1,-1,1.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(23,-1,true,-1,-1,1.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(24,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(25,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(26,-1,true,-1,-1,1.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(27,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(28,-1,true,-1,-1,1.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(29,-1,true,-1,-1,1.0);
            vectNode.push_back(thisNode);
            thisNode = decisiontree::Node(30,-1,true,-1,-1,2.0);
            vectNode.push_back(thisNode);

            pdb::Handle<pdb::Vector<double>> resultMatrix = pdb::makeObject<pdb::Vector<double>>();

            // inference
            double inputValue;
            for (int i = 0; i < inNumRow; i++){
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
              std::cout << treeNode->returnClass << " ";
              resultMatrix->push_back(treeNode->returnClass);
            }
            std::cout << std::endl;
            //std::cout << "Matrix size is: " << resultMatrix->size() << std::endl;
            pdb::Handle<FFMatrixBlock> resultMatrixBlock = pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, inNumRow, 1, resultMatrix);
            return resultMatrixBlock;
        });
    }
};

#endif