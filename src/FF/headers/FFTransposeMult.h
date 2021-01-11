#ifndef FF_TRANSPOSE_MULT_H
#define FF_TRANSPOSE_MULT_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

// LA libraries:
#include <eigen3/Eigen/Dense>

using namespace pdb;

class FFTransposeMult
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFTransposeMult() = default;

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    // return makeLambda(

    // in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
    //   return in1->getBlockColIndex() == in2->getBlockColIndex();
    // });
    // Only this can help with partitioning
    return makeLambdaFromMethod(in1, getBlockColIndex) ==
           makeLambdaFromMethod(in2, getBlockColIndex);

  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [&](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes
            uint32_t I = in1->getRowNums();
            uint32_t J = in2->getRowNums();

            if (in1->getColNums() != in2->getColNums()) {
              std::cerr << "Block dimemsions mismatch! " << std::endl;
              exit(1);
            }

            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockRowIndex(), in2->getBlockRowIndex(), I, J,
                    in1->getTotalRowNums(), in2->getTotalRowNums());

            // get the ptrs
            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();
            double *in1Data = in1->getValue().rawData->c_ptr();
            double *in2Data = in2->getValue().rawData->c_ptr();

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix1(in1Data, in1->getRowNums(), in1->getColNums());
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix2(in2Data, in2->getRowNums(), in2->getColNums());

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                productMatrix(outData, I, J);

            productMatrix = currentMatrix1 * currentMatrix2.transpose();

            return resultFFMatrixBlock;
          } else {
            std::cerr << "Wrong librayCode!" << std::endl;
            exit(1);
          }
        });
  }
};

#endif