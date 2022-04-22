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
    /*std::cout << "SELECTION: in1.blockRowIndex=" << in1->getBlockRowIndex() << std::endl;
    std::cout << "SELECTION: in1.blockColIndex=" << in1->getBlockColIndex() << std::endl;
    std::cout << "SELECTION: in2.blockRowIndex=" << in2->getBlockRowIndex() << std::endl;
    std::cout << "SELECTION: in2.blockColIndex=" << in2->getBlockColIndex() << std::endl;
    std::cout << "SELECTION: in1.rowNums=" << in1->getRowNums() << std::endl;
    std::cout << "SELECTION: in1.colNums=" << in1->getColNums() << std::endl;
    std::cout << "SELECTION: in2.rowNums=" << in2->getRowNums() << std::endl;
    std::cout << "SELECTION: in2.colNums=" << in2->getColNums() << std::endl;*/
    return makeLambdaFromMethod(in1, getBlockColIndex) ==
           makeLambdaFromMethod(in2, getBlockColIndex);

  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [&](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            /*std::cout << "PROJECTION: in1.blockRowIndex=" << in1->getBlockRowIndex() << std::endl;
            std::cout << "PROJECTION: in1.blockColIndex=" << in1->getBlockColIndex() << std::endl;
            std::cout << "PROJECTION: in2.blockRowIndex=" << in2->getBlockRowIndex() << std::endl;
            std::cout << "PROJECTION: in2.blockColIndex=" << in2->getBlockColIndex() << std::endl;
            std::cout << "PROJECTION: in1.rowNums=" << in1->getRowNums() << std::endl;
            std::cout << "PROJECTION: in1.colNums=" << in1->getColNums() << std::endl;
            std::cout << "PROJECTION: in2.rowNums=" << in2->getRowNums() << std::endl;
            std::cout << "PROJECTION: in2.colNums=" << in2->getColNums() << std::endl;*/

	  // get the sizes
            uint32_t I = in1->getRowNums();
            uint32_t J = in2->getRowNums();

            if (in1->getColNums() != in2->getColNums()) {
                std::cout << in1->getColNums() << " " << in2->getColNums() << std::endl;
              std::cerr << "Block dimemsions mismatch! " << std::endl;
              exit(1);
            }
              std::cout << "Checkpoint: TransposeMult" << std::endl;

            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockRowIndex(), in2->getBlockRowIndex(), I, J,
                    in1->getTotalRowNums(), in2->getTotalRowNums(), false);
	    std::cout << "OUTPUT BLOCK[" << in1->getBlockRowIndex() << "," << in2->getBlockRowIndex() <<"]"
		    << ": "<< I << "X" << J << " in " << in1->getTotalRowNums() <<"X" << in2->getTotalRowNums() << std::endl;

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

            // currentMatrix1 [1x6]     currentMatrix2 []
            productMatrix = currentMatrix1 * currentMatrix2.transpose();  // Is this element-element or cross-matmul

            for(int i=0;i<I;i++) {
                for(int j=0;j<J;j++) {
                    int pos = i * I + j;
                    cout << "MatMul: [" << i*I << "," << j << "] " << outData[pos] << " " << in1Data[pos] << " " << in2Data[i + j * I] << endl;
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
