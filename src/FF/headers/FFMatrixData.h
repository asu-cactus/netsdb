#ifndef FF_MATRIXDATA_H
#define FF_MATRIXDATA_H

#include "Handle.h"
#include "Object.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "StringIntPair.h"


class FFMatrixData : public pdb::Object {
public:
  int rowNums = 0;
  int colNums = 0;

  ENABLE_DEEP_COPY

  ~FFMatrixData() {}

  FFMatrixData(int rowNumsIn, int colNumsIn)
      : rowNums(rowNumsIn), colNums(colNumsIn) {
    rawData = pdb::makeObject<pdb::Vector<float>>(rowNumsIn * colNumsIn,
                                                   rowNumsIn * colNumsIn);
  }

  FFMatrixData(int rowNumsIn, int colNumsIn,
               pdb::Handle<pdb::Vector<float>> rawDataIn)
      : rowNums(rowNumsIn), colNums(colNumsIn), rawData(rawDataIn) {
    rawData = pdb::makeObject<pdb::Vector<float>>(rowNumsIn * colNumsIn,
                                                   rowNumsIn * colNumsIn);
  }

  FFMatrixData() {}

  pdb::Handle<pdb::Vector<float>> rawData;
  pdb::Handle<pdb::Vector<float>> bias;

  FFMatrixData &operator+(FFMatrixData &other) {
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
