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
          // get the sizes
          uint32_t I = in1->getRowNums();
          uint32_t K = in1->getColNums();

          std::cout << "Input matrices of size : " << I << " x " << K << " and " << in2->getRowNums() << " x " << in2->getColNums() << " --<GradientJoin>---> " << I << " x " << K << std::endl;

          // make an output
          pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
              pdb::makeObject<FFMatrixBlock>(
                  in1->getBlockRowIndex(), in1->getBlockColIndex(), I, K,
                  in1->getTotalRowNums(), in1->getTotalColNums());

          auto data = resultFFMatrixBlock->getValue().rawData->c_ptr();
          auto lhs = in1->getValue().rawData->c_ptr();
          auto rhs = in2->getValue().rawData->c_ptr();

          // do the multiply
          for (int32_t i = 0; i < I * K; i++) {
            data[i] = lhs[i] * (rhs[i] > 0) * 1.0f;
          }

          // return the output
          return resultFFMatrixBlock;
        });
  }
};

#endif