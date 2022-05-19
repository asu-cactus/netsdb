#ifndef TENSORMETA2D_H
#define TENSORMETA2D_H

#include "Object.h"
#include "Handle.h"


// PRELOAD %TensorMeta2D%

namespace pdb {

class TensorMeta2D : public pdb::Object {
public:
  int blockRowIndex=-1;
  int blockColIndex=-1;
  int totalRows=-1;
  int totalCols=-1;

  ENABLE_DEEP_COPY

  ~TensorMeta2D() {}

  TensorMeta2D() {}
  TensorMeta2D(int rowIndex, int colIndex)
      : blockRowIndex(rowIndex), blockColIndex(colIndex) {}
  TensorMeta2D(int rowIndex, int colIndex, int rows, int cols)
      : blockRowIndex(rowIndex), blockColIndex(colIndex), totalRows(rows),
        totalCols(cols) {}
  
  TensorMeta2D(const TensorMeta2D &rhs) {
    blockColIndex = rhs.blockColIndex;
    blockRowIndex = rhs.blockRowIndex;
    totalCols = rhs.totalCols;
    totalRows = rhs.totalRows;
  }

  bool operator==(const TensorMeta2D &other) const {
    return blockColIndex == other.blockColIndex &&
           blockRowIndex == other.blockRowIndex;
  }

  size_t hash() const override{ 
     return 1000000 * blockRowIndex + blockColIndex; }

  TensorMeta2D &operator=(const TensorMeta2D &rhs) {
    blockColIndex = rhs.blockColIndex;
    blockRowIndex = rhs.blockRowIndex;
    totalCols = rhs.totalCols;
    totalRows = rhs.totalRows;

    return *this;
  }

};

}

#endif
