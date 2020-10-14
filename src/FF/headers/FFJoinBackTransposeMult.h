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
            // std::cout <<"Test Eigen" << std::endl;
            // std::cout <<"Current Matrix1 :"<< std::endl;
            // in1->print();
            // std::cout <<"Current Matrix2 :"<< std::endl;
            // in2->print();
            if (in1->getColNums() != in2->getRowNums()) {
              std::cerr << "Block dimemsions mismatch!" << std::endl;
              exit(1);
            }

            int rowNums = in1->getRowNums();
            int colNums = in2->getColNums();
            int blockRowIndex = in1->getBlockRowIndex();
            int blockColIndex = in2->getBlockColIndex();
            int totalRows = in1->getTotalRowNums();
            int totalCols = in2->getTotalColNums();

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix1(in1->getValue().rawData->c_ptr(),
                               in1->getRowNums(), in1->getColNums());
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix2(in2->getValue().rawData->c_ptr(),
                               in2->getRowNums(), in2->getColNums());

            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(blockRowIndex, blockColIndex,
                                               rowNums, colNums, totalRows,
                                               totalCols);
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                productMatrix(resultFFMatrixBlock->getValue().rawData->c_ptr(),
                              rowNums, colNums);


            // std::cout <<"Result Matrix :"<< std::endl;
            // resultFFMatrixBlock->print();

            // do I need to do some summing here
            if (in1->getBlockColIndex() == (numRows - 1)) {

              // make the bias
              resultFFMatrixBlock->getValue().bias =
                  pdb::makeObject<Vector<double>>(colNums, colNums);
              auto bias = resultFFMatrixBlock->getValue().bias->c_ptr();
              for (auto c = 0; c < colNums; c++) {
                bias[c] = 0.0;
              }

              // sum up the value
              for (auto r = 0; r < in2->getRowNums(); r++) {
                for (auto c = 0; c < in2->getColNums(); c++) {
                  bias[c] += in2->getValue()
                                 .rawData->c_ptr()[r * in2->getColNums() + c];
                }
              }
            }

            productMatrix = currentMatrix1 * currentMatrix2;

            return resultFFMatrixBlock;
          } else {
            std::cerr << "Wrong librayCode!" << std::endl;
            exit(1);
          }
        });
  }
};

#endif