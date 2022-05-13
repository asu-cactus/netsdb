#ifndef FF_MATRIXMETA_H
#define FF_MATRIXMETA_H

#include "Object.h"
#include "Handle.h"

class FFMatrixMeta : public pdb::Object {
public:
  int blockRowIndex=-1;
  int blockColIndex=-1;
  int totalRows=-1;
  int totalCols=-1;
  int distinctBlockId=-1;

  ENABLE_DEEP_COPY

  ~FFMatrixMeta() {}

  FFMatrixMeta() {}
  FFMatrixMeta(int rowIndex, int colIndex)
      : blockRowIndex(rowIndex), blockColIndex(colIndex) {}
  FFMatrixMeta(int rowIndex, int colIndex, int rows, int cols)
      : blockRowIndex(rowIndex), blockColIndex(colIndex), totalRows(rows),
        totalCols(cols) {}
  
  FFMatrixMeta(const FFMatrixMeta &rhs) {
    blockColIndex = rhs.blockColIndex;
    blockRowIndex = rhs.blockRowIndex;
    totalCols = rhs.totalCols;
    totalRows = rhs.totalRows;
    distinctBlockId = rhs.distinctBlockId;
  }

  bool operator==(const FFMatrixMeta &other) const {
    return blockColIndex == other.blockColIndex &&
           blockRowIndex == other.blockRowIndex;
  }

  size_t hash() const override{ 
     //std::cout << "invoke FFMatrixMeta.hash(), blockRowIndex="<<blockRowIndex<<", blockColIndex="<<blockColIndex << std::endl;
     return 1000000 * blockRowIndex + blockColIndex; }

  FFMatrixMeta &operator=(const FFMatrixMeta &rhs) {
    blockColIndex = rhs.blockColIndex;
    blockRowIndex = rhs.blockRowIndex;
    totalCols = rhs.totalCols;
    totalRows = rhs.totalRows;
    distinctBlockId = rhs.distinctBlockId;

    return *this;
  }

};

#endif
