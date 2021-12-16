#ifndef SPECIALIZED_BC_H
#define SPECIALIZED_BC_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
using namespace pdb;

class SpecializedBC: public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  private:
  public:

    ENABLE_DEEP_COPY

    SpecializedBC() {
    }

    Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<FFMatrixBlock> &checkMe) { return true; });
    }

    Lambda<Handle<FFMatrixBlock>>
    getProjection(Handle<FFMatrixBlock> in) override {
        return makeLambda(in, [this](Handle<FFMatrixBlock> &in) {
            // load the metadata
            uint32_t inNumRow = in->getRowNums();
            uint32_t inNumCol = in->getColNums();
            uint32_t inBlockRowIndex = in->getBlockRowIndex();
            uint32_t inBlockColIndex = in->getBlockColIndex();

            // testing purpose
            std::cout << inNumRow << "," << inNumCol << std::endl;
            std::cout << inBlockRowIndex << "," << inBlockColIndex << std::endl;

            double *inData = in->getValue().rawData->c_ptr();

            double texture_mean;
            double smoothness_mean;
            double concave_points_mean;
            double fractal_dimension_mean;
            double area_se;
            double concave_points_se;
            double radius_worst;
            double texture_worst;
            double area_worst;
            double compactness_worst;
            double concavity_worst;

            double result;
            pdb::Handle<pdb::Vector<double>> resultMatrix;

            for (int i = 0; i < inNumRow; i++){
              texture_mean = inData[i*inNumCol+1];
              smoothness_mean = inData[i*inNumCol+4];
              concave_points_mean = inData[i*inNumCol+7];
              fractal_dimension_mean = inData[i*inNumCol+9];
              area_se = inData[i*inNumCol+13];
              concave_points_se = inData[i*inNumCol+17];
              radius_worst = inData[i*inNumCol+20];
              texture_worst = inData[i*inNumCol+21];
              area_worst = inData[i*inNumCol+23];
              compactness_worst = inData[i*inNumCol+25];
              concavity_worst = inData[i*inNumCol+26];
              result = -1.0;

              if(concave_points_mean<=0.052){
                if(radius_worst<=16.54){
                  if(area_se<=37.61){
                    if(texture_worst<=33.27){
                      result = 2.0;
                      break;
                    }else{
                      if(texture_worst<=34.14){
                        result = 1.0;
                        break;
                      }else{
                        result = 2.0;
                        break;
                      }
                    }
                  }else{
                    if(smoothness_mean<=0.091){
                      result = 2.0;
                      break;
                    }else{
                      if(concave_points_se<=0.012){
                        result = 2.0;
                        break;
                      }else{
                        result = 1.0;
                        break;
                      }
                    }
                  }
                }else{
                  if(texture_worst<=20.22){
                    result = 2.0;
                    break;
                  }else{
                    if(concave_points_se<=0.011){
                      result = 1.0;
                      break;
                    }else{
                      result = 2.0;
                      break;
                    }
                  }
                }
              }else{
                if(concavity_worst<=0.225){
                  result = 2.0;
                  break;
                }else{
                  if(area_worst<=710.2){
                    if(texture_worst<=25.95){
                      result = 2.0;
                      break;
                    }else{
                      if(fractal_dimension_mean<=0.065){
                        result = 2.0;
                        break;
                      }else{
                        result = 1.0;
                        break;
                      }
                    }
                  }else{
                    if(texture_mean<=14.12){
                      if(compactness_worst<=0.361){
                        result = 1.0;
                        break;
                      }else{
                        result = 2.0;
                        break;
                      }
                    }else{
                      result = 1.0;
                      break;
                    }
                  }
                }
              }
              resultMatrix->push_back(result);
            }
            pdb::Handle<FFMatrixBlock> resultMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, 1, inBlockRowIndex, 1, resultMatrix);
            return resultMatrixBlock;
        });
    }
};

#endif