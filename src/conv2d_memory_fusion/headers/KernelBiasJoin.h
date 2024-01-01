#ifndef CONV2D_KERNEL_BIAS_JOIN_H
#define CONV2D_KERNEL_BIAS_JOIN_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include <cassert>

using namespace pdb;

namespace conv2d_memory_fusion {
class KernelBiasJoin
    : public JoinComp<::FFMatrixBlock, ::FFMatrixBlock, ::FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  KernelBiasJoin() {}

  Lambda<bool> getSelection(Handle<::FFMatrixBlock> in1,
                            Handle<::FFMatrixBlock> in2) override {
    return makeLambdaFromMethod(in1, getBlockRowIndex) ==
               makeLambdaFromMethod(in2, getBlockRowIndex) &&
           makeLambdaFromMethod(in1, isLastColBlock);
  }

  Lambda<Handle<::FFMatrixBlock>>
  getProjection(Handle<::FFMatrixBlock> in1,
                Handle<::FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2,
        [](Handle<::FFMatrixBlock> &in1, Handle<::FFMatrixBlock> &in2) {
          // in1 is kernel, in2 is bias
	  std::cout << "in2->getColNums()=" << in2->getColNums() << std::endl;
	  std::cout << "in2->getTotalColNums()=" << in2->getTotalColNums() << std::endl;
	  std::cout << "in2->getRowNums()=" << in2->getRowNums() << std::endl;
	  std::cout << "in1->getRowNums()=" << in1->getRowNums() << std::endl;
	  std::cout << "in1->getTotalRowNums()=" << in1->getTotalRowNums() << std::endl;
	  std::cout << "in1->getColNums()=" << in1->getColNums() << std::endl;
	  std::cout << "in1->getTotalColNums()=" << in1->getTotalColNums() << std::endl;
	  std::cout << "in2->isLastColBlock()=" <<in2->isLastColBlock() << std::endl;
          assert(in2->getColNums() == 1 && in2->getTotalColNums() == 1 &&
                 in1->getTotalRowNums() == in2->getRowNums() &&
                 in2->isLastColBlock());

          float *in1Data = in1->getValue().rawData->c_ptr();
          float *in2Data = in2->getValue().rawData->c_ptr();
          int pos = in1->getTotalColNums() % in1->getColNums() - 1;

          // Copy bias values
          for (int i = 0; i < in1->getRowNums(); i++) {
            in1Data[i * in1->getColNums() + pos] = in2Data[i];
          }

          return in1;
        });
  }
};
} // namespace conv2d_memory_fusion

#endif
