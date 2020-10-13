#ifndef FF_GRADIENT_JOIN_H
#define FF_GRADIENT_JOIN_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

class FFGradientJoin
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFGradientJoin() {}

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          return in1->getKey() == in2->getKey();
        });
  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          int rowNums = in1->getRowNums();
          int colNums = in2->getColNums();
          int blockRowIndex = in1->getBlockRowIndex();
          int blockColIndex = in2->getBlockColIndex();
          int totalRows = in1->getTotalRowNums();
          int totalCols = in2->getTotalColNums();

          pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
              pdb::makeObject<FFMatrixBlock>(blockRowIndex, blockColIndex,
                                             rowNums, colNums, totalRows,
                                             totalCols);

          auto data = resultFFMatrixBlock->getValue().rawData->c_ptr();
          auto lhs = in1->getValue().rawData->c_ptr();
          auto rhs = in2->getValue().rawData->c_ptr();

          // do the multiply
          for (int32_t i = 0; i < in1->getRowNums() * in1->getColNums(); i++) {
            data[i] = lhs[i] * (rhs[i] > 0) * 1.0f;
          }

          return resultFFMatrixBlock;
        });
  }
};

#endif