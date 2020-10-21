#ifndef FF_HIDDENLAYER_JOIN_H
#define FF_HIDDENLAYER_JOIN_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

// LA libraries:
#include <eigen3/Eigen/Dense>

using namespace pdb;

class FFHiddenLayerJoin
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFHiddenLayerJoin() = default;

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          return in1->getBlockColIndex() == in2->getBlockRowIndex();
        });
  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes
            uint32_t I = in1->getRowNums();
            uint32_t J = in2->getColNums();
            uint32_t K = in1->getColNums();

            // make an output
            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockRowIndex(), in2->getBlockColIndex(), I, J,
                    in1->getTotalRowNums(), in2->getTotalColNums());

            std::cout << "[FFHiddenLayerJoin] " << in1->getBlockRowIndex() << ", " << in1->getBlockColIndex() << " X " << in2->getBlockRowIndex() << ", " << in2->getBlockColIndex() << " = " << in1->getBlockRowIndex() << ", " << in2->getBlockColIndex() << std::endl;

            // get the ptrs
            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();
            double *in1Data = in1->getValue().rawData->c_ptr();
            double *in2Data = in2->getValue().rawData->c_ptr();

            // do the relu on the activation this is left from the previous
            // iteration
            for (int32_t i = 0; i < I * K; i++) {
              if (in1Data[i] < 0) {
                in1Data[i] = 0;
              }
            }

            // do the multiply
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix1(in1->getValue().rawData->c_ptr(),
                               in1->getRowNums(), in1->getColNums());
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix2(in2->getValue().rawData->c_ptr(),
                               in2->getRowNums(), in2->getColNums());

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                productMatrix(resultFFMatrixBlock->getValue().rawData->c_ptr(),
                              I, J);

            productMatrix = currentMatrix1 * currentMatrix2;

            // process the bias if necessary
            if (in2->getValue().bias != nullptr) {
              auto bias = in2->getValue().bias->c_ptr();
              for (uint32_t r = 0; r < I; r++) {
                for (uint32_t c = 0; c < J; c++) {

                  // add the bias
                  outData[r * J + c] += bias[c];
                }
              }
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