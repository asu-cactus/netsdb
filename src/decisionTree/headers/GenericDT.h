#ifndef GENERIC_DT_H
#define GENERIC_DT_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

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

class GenericDT: public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  private:
    decisiontree::Node * ptr = nullptr;

  public:

    ENABLE_DEEP_COPY

    GenericDT() {}
    ~GenericDT() {}

    GenericDT(decisiontree::Node &ptrIn) {
      this->ptr = &ptrIn;
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
            std::cout << inNumRow << "," << inNumCol << std::endl;
            std::cout << inBlockRowIndex << "," << inBlockColIndex << std::endl;

            double *inData = in->getValue().rawData->c_ptr();

            pdb::Handle<pdb::Vector<double>> resultMatrix = pdb::makeObject<pdb::Vector<double>>();

            for (int i = 0; i < inNumRow; i++){
              while(ptr->isLeaf == false){
                double inputValue = inData[i*inNumRow+ptr->indexID];
                if(inputValue <= ptr->returnClass){
                  *ptr = *(ptr+(ptr->leftChild));
                }else{
                  *ptr = *(ptr+(ptr->rightChild));
                }
              }
              resultMatrix->push_back(ptr->returnClass);
            }
            pdb::Handle<FFMatrixBlock> resultMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, inNumRow, 1, resultMatrix);
            return resultMatrixBlock;
        });
    }
};

#endif