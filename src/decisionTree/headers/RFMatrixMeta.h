#ifndef RF_MATRIXMETA_H
#define RF_MATRIXMETA_H

#include "Object.h"
#include "Handle.h"

class RFMatrixMeta : public pdb::Object {
public:
  int blockRowIndex;
  int blockColIndex;
  int totalRows;
  int totalCols;

  ENABLE_DEEP_COPY

  ~RFMatrixMeta() {}

  RFMatrixMeta() {}
  RFMatrixMeta(int rowIndex, int colIndex)
      : blockRowIndex(rowIndex), blockColIndex(colIndex) {}
  RFMatrixMeta(int rowIndex, int colIndex, int rows, int cols)
      : blockRowIndex(rowIndex), blockColIndex(colIndex), totalRows(rows),
        totalCols(cols) {}
  
  RFMatrixMeta(const RFMatrixMeta &rhs) {
    blockColIndex = rhs.blockColIndex;
    blockRowIndex = rhs.blockRowIndex;
    totalCols = rhs.totalCols;
    totalRows = rhs.totalRows;
  }

  bool operator==(const RFMatrixMeta &other) const {
    return blockColIndex == other.blockColIndex &&
           blockRowIndex == other.blockRowIndex;
  }

  size_t hash() const override{ 
     return 10000 * blockRowIndex + blockColIndex; }

  RFMatrixMeta &operator=(const RFMatrixMeta &rhs) {
    blockColIndex = rhs.blockColIndex;
    blockRowIndex = rhs.blockRowIndex;
    totalCols = rhs.totalCols;
    totalRows = rhs.totalRows;

    return *this;
  }

};

#endif