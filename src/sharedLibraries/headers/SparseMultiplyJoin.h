#ifndef SILLY_LA_MULTIPLY1_JOIN_H
#define SILLY_LA_MULTIPLY1_JOIN_H

// by Binhang, May 2017

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "JoinComp.h"
//#include "BuiltInMatrixBlock.h"
#include "MatrixSparse.h"
#include <mkl.h>
// LA libraries:
#include <eigen3/Eigen/Dense>

using namespace pdb;

class SparseMultiplyJoin : public JoinComp<MatrixSparse,MatrixSparse, MatrixSparse> {

public:
    ENABLE_DEEP_COPY

    SparseMultiplyJoin() {}

    Lambda<bool> getSelection(Handle<MatrixSparse> in1, Handle<MatrixSparse> in2) override {
        return makeLambdaFromMethod(in1, getBlockColIndex) ==
            makeLambdaFromMethod(in2,
                                 getBlockRowIndex);  // This can be recognized by the pdb optimizer.
    }


    Lambda<Handle<MatrixSparse>> getProjection(Handle<MatrixSparse> in1,
                                              Handle<MatrixSparse> in2) override {
        return makeLambda(in1, in2, [](Handle<MatrixSparse>& in1, Handle<MatrixSparse>& in2) {
                if (in1->getColNums() != in2->getRowNums()) {
                    std::cerr << "Block dimemsions mismatch!" << std::endl;
                    exit(1);
                }
                int rowNums = in1->rowNumsIn;
                int colNums = in2->colNumsIn;
                int blockRowIndex = in1->getBlockRowIndex();
                int blockColIndex = in2->getBlockColIndex();
                int totalRows = in1->getTotalRowNums();
                int totalCols = in2->getTotalColNums();
                float *values;
            sparse_matrix_t csrA = nullptr;
     	     sparse_matrix_t csrB = nullptr;

	      // create the csr matrix
	    mkl_sparse_s_create_csr(&csrA, SPARSE_INDEX_BASE_ZERO, in1->rowNumsIn, in1->colNumsIn, in1->rowIndices.c_ptr(), in1->rowIndices.c_ptr() + 1, in1->colIndices.c_ptr(), in1->values.c_ptr());
      	    mkl_sparse_s_create_csr(&csrB, SPARSE_INDEX_BASE_ZERO, in2->rowNumsIn, in2->colNumsIn, in2->rowIndices.c_ptr(), in2->rowIndices.c_ptr() + 1, in2->colIndices.c_ptr(), in2->values.c_ptr());
        
           pdb::Handle<MatrixSparse> resultMatrixBlock = pdb::makeObject<MatrixSparse>(
      
              blockRowIndex, blockColIndex, rowNums, colNums, totalRows, totalCols);
            mkl_sparse_s_spmmd(SPARSE_OPERATION_NON_TRANSPOSE, csrA, csrB, SPARSE_LAYOUT_ROW_MAJOR, values, colNums);
              
		 memcpy(resultMatrixBlock->getRawDataHandle()->c_ptr(),values,sizeof(double)*colNums);

                return resultMatrixBlock;
            
        });
    }
};


#endif
