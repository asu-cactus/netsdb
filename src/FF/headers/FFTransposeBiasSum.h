#ifndef FF_TRANSPOSE_BIAS_SUM_H
#define FF_TRANSPOSE_BIAS_SUM_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

// LA libraries:
#include <eigen3/Eigen/Dense>
#include <cmath>

using namespace pdb;

class FFTransposeBiasSum
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFTransposeBiasSum() = default;

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          std::cout << "[FFTRANBIASSUM] " <<  in1->getBlockRowIndex() << ", " << in1->getBlockColIndex() << " : " << in2->getBlockRowIndex() << ", " << in2->getBlockColIndex() << std::endl;
            return in1->getBlockRowIndex() == in2->getBlockRowIndex()
                && in1->getBlockColIndex() == in2->getBlockColIndex();
        });
  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [&](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes

            uint32_t I = in1->getRowNums();
            uint32_t J = in1->getColNums();

            std::cout << "[FFTRANBIASSUM] Summing!" << std::endl;

            if (in1->getRowNums() != in2->getRowNums() ||
                in1->getColNums() != in2->getColNums()) {
                std::cerr << "Block dimemsions mismatch!" << std::endl;
                exit(1);
            }

            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockColIndex(), in1->getBlockRowIndex(), J, I,
                    in1->getTotalColNums(), in1->getTotalRowNums());

            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();
            double *in1Data = in1->getValue().rawData->c_ptr();
            double *in2Data = in2->getValue().rawData->c_ptr();

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                currentMatrix1(in1Data, I, J);
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                currentMatrix2(in2Data, I, J);

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
                sumMatrix(outData, I, J);

            sumMatrix = currentMatrix1 + currentMatrix2;
            sumMatrix.transposeInPlace();

            double sum = 0;
            for (int i = 0; i < I * J; i++) {
              outData[i] = exp(outData[i]);
              sum += outData[i];
            }

            sumMatrix /= sum;

            return resultFFMatrixBlock;
          } else {
            std::cerr << "Wrong librayCode!" << std::endl;
            exit(1);
          }
        });
  }
};

#endif