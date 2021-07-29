#ifndef FF_ACTIVATION_BIAS_SUM_H
#define FF_ACTIVATION_BIAS_SUM_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include <cstdlib>
#include <ctime>

using namespace pdb;

class FFActivationBiasSum
    : public JoinComp<FFMatrixBlock, FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

enum class SumActivation { Relu = 1, Sigmod };

  SumActivation modifier;

  FFActivationBiasSum() : modifier(SumActivation::Relu), dropout_rate(0) {};

  FFActivationBiasSum(SumActivation modifier, double dropout_rate_in=0) : modifier(modifier), dropout_rate(dropout_rate_in){};

  double dropout_rate;

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2) override {
    return makeLambdaFromMethod(in1, getBlockRowIndex) ==
           makeLambdaFromMethod(in2, getBlockRowIndex);

  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(
        in1, in2, [&](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes

            uint32_t I = in1->getRowNums();
            uint32_t J = in1->getColNums();

            if (in1->getRowNums() != in2->getRowNums() ||
                in2->getColNums() != 1) {
              std::cout << "[FFActivationBiasSum] IN1 : " << I << " X " << J
                        << ", IN2: " << in2->getRowNums() << " X "
                        << in2->getColNums() << std::endl;
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

            srand(time(NULL));
            for (int32_t i = 0; i < I; i++) {
              for (int32_t j = 0; j < J; j++) {
                int32_t pos = i * J + j;

              switch (modifier) {
              case SumActivation::Sigmod:
                outData[pos] = 1 / (1 + exp(-1 * (in1Data[pos] + in2Data[i])));
                break;

              case SumActivation::Relu:
                outData[i] = max(0.0, in1Data[pos] + in2Data[i]);
                break;

              default:
                std::cerr << "Invalid modifier!" << std::endl;
                exit(1);
              }

                if (dropout_rate != 0) {
                  bool zero = (rand() % 100) < (dropout_rate * 100);
                  if (zero) {
                    outData[pos] = 0;
                  } else {
                    outData[pos] *= 1 / (1 - dropout_rate);
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
