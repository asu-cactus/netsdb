#ifndef FF_TRANSPOSE_BIAS_SUM_SIGMOID_H
#define FF_TRANSPOSE_BIAS_SUM_SIGMOID_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

// LA libraries:
#include <cmath>
#include <eigen3/Eigen/Dense>

using namespace pdb;

class FFTransposeBiasSumSigmoid
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFTransposeBiasSumSigmoid() = default;

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    // return makeLambda(

    //     in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2)
    //     {
    //       return in1->getBlockRowIndex() == in2->getBlockRowIndex();
    //     });
    return makeLambdaFromMethod(in1, getBlockRowIndex) ==
           makeLambdaFromMethod(in2, getBlockRowIndex);

  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [&](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes
            /*std::cout << "in1.blockRowIndex=" << in1->getBlockRowIndex() << std::endl;
            std::cout << "in1.blockColIndex=" << in1->getBlockColIndex() << std::endl;
            std::cout << "in2.blockRowIndex=" << in2->getBlockRowIndex() << std::endl;
            std::cout << "in2.blockColIndex=" << in2->getBlockColIndex() << std::endl;
            std::cout << "in1.blockRowIndex=" << in1->getBlockRowIndex() << std::endl;
            std::cout << "in1.blockColIndex=" << in1->getBlockColIndex() << std::endl;
            std::cout << "in1.rowNums=" << in1->getRowNums() << std::endl;
            std::cout << "in1.colNums=" << in1->getColNums() << std::endl;
            std::cout << "in2.rowNums=" << in2->getRowNums() << std::endl;
            std::cout << "in2.colNums=" << in2->getColNums() << std::endl;*/
            uint32_t I = in1->getRowNums();//5000
            uint32_t J = in1->getColNums(); //1
            
            if (in1->getRowNums() != in2->getRowNums() ||
                in2->getColNums() != 1) {
              std::cerr << "Block dimemsions mismatch!" << std::endl;
              exit(1);
            }

            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockRowIndex(), in1->getBlockColIndex(), I, J,
                    in1->getTotalRowNums(), in1->getTotalColNums(), false);

            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();
            double *in1Data = in1->getValue().rawData->c_ptr();
            double *in2Data = in2->getValue().rawData->c_ptr();

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                currentMatrix1(in1Data, I, J);
            Eigen::Map<
                Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::ColMajor>>
                currentMatrix2(in2Data, I, 1);

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                sumMatrix(outData, I, J);

            sumMatrix = currentMatrix1 + currentMatrix2;
            //sumMatrix.transposeInPlace();

            int row_idx = in1->getBlockRowIndex();//0
            int col_idx = in1->getBlockColIndex();//0
            int block_x = I;//5000
            int block_y = J;//1
            int X = in1->getTotalRowNums();
            int Y = in1->getTotalColNums();

            for (int i = 0; i < block_x; i++) {
              int act_x = row_idx * block_x + i;
              for (int j = 0; j < block_y; j++) {
                int act_y = col_idx * block_y + j;

                if (act_x < X && act_y < Y) {
                  int cur_pos = i * block_y + j;
                  double new_value = (double)exp(outData[cur_pos]);
		  new_value = new_value/((double)1.0+new_value);

		  if (new_value >=0.5) {
		     outData[cur_pos] = 1;
		  } else {
		     outData[cur_pos] = 0;
		  }
		  
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
