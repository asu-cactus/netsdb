#ifndef LSTM_THREE_WAY_SUM_H
#define LSTM_THREE_WAY_SUM_H

#include "FFMatrixBlock.h"
#include "JoinComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include <cmath>

using namespace pdb;

enum class SumActivation { Sigmod = 1, Tanh };

class LSTMThreeWaySum : public JoinComp<FFMatrixBlock, FFMatrixBlock,
                                        FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  SumActivation modifier;

  LSTMThreeWaySum() : modifier(SumActivation::Sigmod){};

  LSTMThreeWaySum(SumActivation modifierIn) : modifier(modifierIn){};

  Lambda<bool> getSelection(Handle<FFMatrixBlock> in1,
                            Handle<FFMatrixBlock> in2,
                            Handle<FFMatrixBlock> in3) override {
    return ((makeLambdaFromMethod(in1, getKey) ==
             makeLambdaFromMethod(in2, getKey)) &&
            (makeLambdaFromMethod(in1, getKey) ==
             makeLambdaFromMethod(in3, getKey)));
  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> in1, Handle<FFMatrixBlock> in2,
                Handle<FFMatrixBlock> in3) override {
    return makeLambda(
        in1, in2, in3,
        [this](Handle<FFMatrixBlock> &in1, Handle<FFMatrixBlock> &in2,
               Handle<FFMatrixBlock> &in3) {
          if (FFMatrixBlock::librayCode == EIGEN_CODE) {
            // get the sizes

            uint32_t I = in1->getRowNums();
            uint32_t J = in1->getColNums();

            std::cout << "Multiplying" << in1->getBlockRowIndex() << " x "
                      << in1->getBlockColIndex() << std::endl;

            if (in1->getRowNums() != in2->getRowNums() ||
                in1->getColNums() != in2->getColNums() ||
                in2->getRowNums() != in3->getRowNums() ||
                in2->getColNums() != in3->getColNums()) {
              std::cerr << "Block dimemsions mismatch!" << std::endl;
              exit(1);
            }

            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(
                    in1->getBlockRowIndex(), in1->getBlockColIndex(), I, J,
                    in1->getTotalRowNums(), in1->getTotalColNums());

            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();
            double *in1Data = in1->getValue().rawData->c_ptr();
            double *in2Data = in2->getValue().rawData->c_ptr();
            double *in3Data = in3->getValue().rawData->c_ptr();

            for (int i = 0; i < I * J; i++) {
              double sum = (in1Data[i] + in2Data[i] + in3Data[i]);

              switch (modifier) {
              case SumActivation::Sigmod:
                outData[i] = 1 / (1 + exp(-1 * sum));
                break;

              case SumActivation::Tanh:
                outData[i] =
                    (exp(sum) - exp(-1 * sum)) / (exp(sum) + exp(-1 * sum));
                break;

              default:
                std::cerr << "Invalid modifier!" << std::endl;
                exit(1);
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