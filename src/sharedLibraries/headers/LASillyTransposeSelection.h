#ifndef SILLY_LA_TRANSPOSE_SELECT_H
#define SILLY_LA_TRANSPOSE_SELECT_H

// by Binhang, May 2017

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
//#include "BuiltInMatrixBlock.h"
#include "MatrixBlock.h"

// LA libraries:
#include <eigen3/Eigen/Dense>

using namespace pdb;

class LASillyTransposeSelection : public SelectionComp<MatrixBlock, MatrixBlock> {

public:
    ENABLE_DEEP_COPY

    LASillyTransposeSelection() {}

    Lambda<bool> getSelection(Handle<MatrixBlock> checkMe) override {
        return makeLambda(checkMe, [](Handle<MatrixBlock>& checkMe) { return true; });
    }


    Lambda<Handle<MatrixBlock>> getProjection(Handle<MatrixBlock> checkMe) override {
        return makeLambda(checkMe, [](Handle<MatrixBlock>& checkMe) {
            if (MatrixBlock::librayCode == EIGEN_CODE) {
                // std::cout <<"Test Eigen" << std::endl;
                // std::cout <<"Current Matrix :"<< std::endl;
                // checkMe->print();
                int rowNums = checkMe->getRowNums();
                int colNums = checkMe->getColNums();
                int blockRowIndex = checkMe->getBlockRowIndex();
                int blockColIndex = checkMe->getBlockColIndex();
                int totalRows = checkMe->getTotalColNums();
                int totalCols = checkMe->getTotalRowNums();
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                    currentMatrix(checkMe->getRawDataHandle()->c_ptr(), rowNums, colNums);

                // std::cout <<"Test Safe ?" << std::endl;
                // std::cout << "Eigen matrix:\n" << currentMatrix << std::endl;
                pdb::Handle<MatrixBlock> resultMatrixBlock = pdb::makeObject<MatrixBlock>(
                    blockColIndex, blockRowIndex, colNums, rowNums, totalRows, totalCols);

                // std::cout <<"Test Safe ??" << std::endl;
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                    transposeMatrix(
                        resultMatrixBlock->getRawDataHandle()->c_ptr(), colNums, rowNums);
                // std::cout <<"Test Safe ???" << std::endl;
                transposeMatrix = currentMatrix.transpose();
                // std::cout << "Eigen matrix:\n" << transposeMatrix << std::endl;
                // std::cout <<"Transposed Matrix :"<< std::endl;
                // resultMatrixBlock->print();

                return resultMatrixBlock;
            } else {
                std::cerr << "Wrong librayCode!" << std::endl;
                exit(1);
            }
        });
    }
};


#endif
