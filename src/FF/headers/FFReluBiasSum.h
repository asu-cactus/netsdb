#ifndef FF_RELU_BIAS_SUM_H
#define FF_RELU_BIAS_SUM_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

class FFReluBiasSum
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFReluBiasSum() = default;

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
            return in1->getBlockRowIndex() == in2->getBlockRowIndex()
                && in1->getBlockColIndex() == in2->getBlockColIndex();
        });
  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [&](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes

            uint32_t I = in1->getRowNums();
            uint32_t J = in1->getColNums();

            if (in1->getRowNums() != in2->getRowNums() ||
                in1->getColNums() != in2->getColNums()) {
                std::cerr << "Block dimemsions mismatch!" << std::endl;
                exit(1);
            }

            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockRowIndex(), in1->getBlockColIndex(), I, J,
                    in1->getTotalRowNums(), in1->getTotalColNums());

            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();
            double *in1Data = in1->getValue().rawData->c_ptr();
            double *in2Data = in2->getValue().rawData->c_ptr();

            for (int32_t i = 0; i < I * J; i++) {
              outData[i] = in1Data[i] + in2Data[i];
              outData[i] = outData[i] < 0 ? 0 : outData[i];
            }

            return resultFFMatrixBlock;
          } else {
            std::cerr << "Wrong librayCode!" << std::endl;
            exit(1);
          }
        });
  }
};

#endif