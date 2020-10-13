#ifndef FF_MATRIXDATA_H
#define FF_MATRIXDATA_H

#include "MatrixData.h"

class FFMatrixData : public MatrixData {
public:
  ENABLE_DEEP_COPY

  ~FFMatrixData() {}
  FFMatrixData() {}

  pdb::Handle<pdb::Vector<double>> bias;

  FFMatrixData &operator+(FFMatrixData &other) {
    double *myData, *otherData;
    setSumFlag();

    MatrixData::operator+(static_cast<MatrixData &>(other));

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