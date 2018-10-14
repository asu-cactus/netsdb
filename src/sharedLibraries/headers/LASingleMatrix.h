#ifndef LA_SINGLE_MATRIX_H
#define LA_SINGLE_MATRIX_H

#include "Object.h"
#include "Handle.h"
#include "MatrixBlock.h"
#include <eigen3/Eigen/Dense>

// By Binhang, June 2017
// This is only used by a temporal solution of matrix inverse.


using namespace pdb;
class SingleMatrix : public Object {
private:
    int key = 1;  // Any constant should work here!
    MatrixBlock myValue;

public:
    ENABLE_DEEP_COPY

    ~SingleMatrix() {}
    SingleMatrix() {}

    SingleMatrix(MatrixBlock block) : key(1), myValue(block) {}

    size_t hash() const {
        return myValue.hash();
    }


    int& getKey() {
        return key;
    }

    MatrixBlock& getValue() {
        return myValue;
    }

    void print() {
        std::cout << "Single matrix" << std::endl;
        myValue.print();
    }

    pdb::Handle<SingleMatrix> getInverse() {
        if (MatrixBlock::librayCode == EIGEN_CODE) {
            // std::cout <<"Test Eigen" << std::endl;
            // std::cout <<"Current Matrix :"<< std::endl;
            // this->print();
            int rowNums = myValue.getRowNums();
            int colNums = myValue.getColNums();
            int blockRowIndex = myValue.getBlockRowIndex();
            int blockColIndex = myValue.getBlockColIndex();
            int totalRows = myValue.getTotalRowNums();
            int totalCols = myValue.getTotalColNums();
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                currentMatrix(myValue.getRawDataHandle()->c_ptr(), rowNums, colNums);

            // std::cout <<"Test Safe ?" << std::endl;
            // std::cout << "Eigen matrix:\n" << currentMatrix << std::endl;
            pdb::Handle<MatrixBlock> resultMatrixBlock = pdb::makeObject<MatrixBlock>(
                blockColIndex, blockRowIndex, colNums, rowNums, totalRows, totalCols);

            // std::cout <<"Test Safe ??" << std::endl;
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                inverseMatrix(resultMatrixBlock->getRawDataHandle()->c_ptr(), colNums, rowNums);
            // std::cout <<"Test Safe ???" << std::endl;
            inverseMatrix = currentMatrix.inverse();
            // std::cout << "Eigen matrix:\n" << inverseMatrix << std::endl;
            // std::cout <<"Inverse Matrix :"<< std::endl;
            // resultMatrixBlock->print();

            pdb::Handle<SingleMatrix> result = pdb::makeObject<SingleMatrix>(*resultMatrixBlock);
            return result;
        } else {
            std::cerr << "Wrong librayCode!" << std::endl;
            exit(1);
        }
    }
};


#endif
