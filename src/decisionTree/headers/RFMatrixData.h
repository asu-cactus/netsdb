#ifndef RF_MATRIXDATA_H
#define RF_MATRIXDATA_H

#include "Handle.h"
#include "Object.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "StringIntPair.h"


class RFMatrixData : public pdb::Object {
public:
  int rowNums = 0;
  int colNums = 0;

  ENABLE_DEEP_COPY

  ~RFMatrixData() {}

  RFMatrixData(int rowNumsIn, int colNumsIn)
      : rowNums(rowNumsIn), colNums(colNumsIn) {
    rawData = pdb::makeObject<pdb::Vector<float>>(rowNumsIn * colNumsIn,
                                                   rowNumsIn * colNumsIn);
  }

  RFMatrixData(int rowNumsIn, int colNumsIn,
               pdb::Handle<pdb::Vector<float>> rawDataIn)
      : rowNums(rowNumsIn), colNums(colNumsIn), rawData(rawDataIn) {
    rawData = pdb::makeObject<pdb::Vector<float>>(rowNumsIn * colNumsIn,
                                                   rowNumsIn * colNumsIn);
  }

  RFMatrixData() {}

  pdb::Handle<pdb::Vector<float>> rawData;
  pdb::Handle<pdb::Vector<float>> bias;

  RFMatrixData &operator+(RFMatrixData &other) {
    float *myData, *otherData;

    myData = rawData->c_ptr();
    otherData = other.rawData->c_ptr();

    for (int i = 0; i < rowNums * colNums; i++) {
      (myData)[i] += (otherData)[i];
    }

    if (bias != nullptr && other.bias != nullptr) {
      myData = bias->c_ptr();
      otherData = other.bias->c_ptr();
      for (int i = 0; i < other.bias->size(); i++) {
        (myData)[i] += (otherData)[i];
      }
    }

    return *this;
  }
};

#endif