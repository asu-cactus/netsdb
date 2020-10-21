#ifndef FF_JOIN_BACK_TRANSPOSE_MULT_H
#define FF_JOIN_BACK_TRANSPOSE_MULT_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

// LA libraries:
#include <eigen3/Eigen/Dense>

using namespace pdb;

class FFJoinBackTransposeMult
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFJoinBackTransposeMult() = default;
  explicit FFJoinBackTransposeMult(uint32_t numRows) : numRows(numRows){};

  uint32_t numRows;

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          return in1->getBlockRowIndex() == in2->getBlockRowIndex();
        });
  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [&](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes
            uint32_t I = in1->getColNums();
            uint32_t J = in2->getColNums();
            uint32_t K = in1->getRowNums();

            if (in1->getRowNums() != in2->getRowNums()) {
              std::cerr << "Block dimemsions mismatch! " << in1->getRowNums()
                        << " x " << in1->getColNums() << " vs "
                        << in2->getRowNums() << " x " << in2->getColNums()
                        << std::endl;
              exit(1);
            }

            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockColIndex(), in2->getBlockColIndex(), I, J,
                    in1->getTotalColNums(), in2->getTotalColNums());

            // get the ptrs
            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();
            double *in1Data = in1->getValue().rawData->c_ptr();
            double *in2Data = in2->getValue().rawData->c_ptr();

            // do I need to do some summing here
            if (in1->getBlockColIndex() == (numRows - 1)) {

              // make the bias
              resultFFMatrixBlock->getValue().bias =
                  pdb::makeObject<pdb::Vector<double>>(J, J);
              auto bias = resultFFMatrixBlock->getValue().bias->c_ptr();
              for (auto c = 0; c < J; c++) {
                bias[c] = 0.0f;
              }

              // sum up the value
              for (auto r = 0; r < in2->getRowNums(); r++) {
                for (auto c = 0; c < in2->getColNums(); c++) {
                  bias[c] += in2Data[r * in2->getColNums() + c];
                }
              }
            }

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

            productMatrix = currentMatrix1.transpose() * currentMatrix2;

            return resultFFMatrixBlock;
          } else {
            std::cerr << "Wrong librayCode!" << std::endl;
            exit(1);
          }
        });
  }
};

#endif