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

            // do the relu on the activation this is left from the previous iteration
            for(int32_t i = 0; i < rowNums * in1->getColNums(); i++) {
                if(in1->getValue().rawData->c_ptr()[i] < 0) { in1->getValue().rawData->c_ptr()[i] = 0; }
            }

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

            productMatrix = currentMatrix1 * currentMatrix2;

            // std::cout <<"Result Matrix :"<< std::endl;
            // resultFFMatrixBlock->print();

            // process the bias if necessary
            if (in2->getValue().bias != nullptr) {
              auto bias = in2->getValue().bias->c_ptr();
              resultFFMatrixBlock->getValue().bias =
                  makeObject<pdb::Vector<double>>(rowNums, colNums);

              for (uint32_t r = 0; r < rowNums; r++) {
                for (uint32_t c = 0; c < colNums; c++) {

                  // add the bias
                  resultFFMatrixBlock->getValue()
                      .bias->c_ptr()[r * colNums + c] += bias[c];
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