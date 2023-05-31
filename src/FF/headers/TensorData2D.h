#ifndef TENSORDATA2D_H
#define TENSORDATA2D_H

#include "Handle.h"
#include "Object.h"
#include "PDBString.h"
#include "PDBVector.h"


namespace pdb {

template <class T>
class TensorData2D : public pdb::Object {
public:
  int rowNums = 0;
  int colNums = 0;

  ENABLE_DEEP_COPY

  ~TensorData2D() {}

  TensorData2D(int rowNumsIn, int colNumsIn)
      : rowNums(rowNumsIn), colNums(colNumsIn) {
    rawData = pdb::makeObject<pdb::Vector<T>>(rowNumsIn * colNumsIn,
                                                   rowNumsIn * colNumsIn);
  }

  TensorData2D(int rowNumsIn, int colNumsIn,
               pdb::Handle<pdb::Vector<T>> rawDataIn)
      : rowNums(rowNumsIn), colNums(colNumsIn), rawData(rawDataIn) {
    rawData = pdb::makeObject<pdb::Vector<T>>(rowNumsIn * colNumsIn,
                                                   rowNumsIn * colNumsIn);
  }

  TensorData2D() {}

  pdb::Handle<pdb::Vector<T>> rawData;
  pdb::Handle<pdb::Vector<T>> bias;

  TensorData2D &operator+(TensorData2D &other) {
    T *myData, *otherData;

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

}

#endif
