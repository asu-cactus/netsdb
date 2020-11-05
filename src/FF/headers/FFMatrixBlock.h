#ifndef FF_MATRIXBLOCK_H
#define FF_MATRIXBLOCK_H

#ifndef EIGEN_CODE
#define EIGEN_CODE 0
#endif

#include "FFMatrixData.h"
#include "FFMatrixMeta.h"
#include "Handle.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "StringIntPair.h"

// LA libraries:
#include <eigen3/Eigen/Dense>

class FFMatrixBlock : public pdb::Object {
private:
  FFMatrixData data;
  FFMatrixMeta meta;

public:
  const static int librayCode = EIGEN_CODE;

  ENABLE_DEEP_COPY

  ~FFMatrixBlock() {}

  FFMatrixBlock() {}

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn)
      : data(rowNumsIn, colNumsIn), meta(blockRowIndexIn, blockColIndexIn) {}

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols)
      : data(rowNumsIn, colNumsIn),
        meta(blockRowIndexIn, blockColIndexIn, rowNumsIn, colNumsIn) {}

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, pdb::Handle<pdb::Vector<double>> rawDataIn)
      : data(rowNumsIn, colNumsIn, rawDataIn),
        meta(blockRowIndexIn, blockColIndexIn) {}

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols,
                pdb::Handle<pdb::Vector<double>> rawDataIn)
      : data(rowNumsIn, colNumsIn, rawDataIn),
        meta(blockRowIndexIn, blockColIndexIn, rowNumsIn, colNumsIn) {}

  int getRowNums() { return data.rowNums; }

  int getColNums() { return data.colNums; }

  pdb::Handle<pdb::Vector<double>> &getRawDataHandle() { return data.rawData; }

  FFMatrixMeta &getKey() { return meta; }

  FFMatrixData &getValue() { return data; }

  int getBlockRowIndex() { return meta.blockRowIndex; }

  int getBlockColIndex() { return meta.blockColIndex; }

  int getTotalRowNums() { return meta.totalRows; }

  int getTotalColNums() { return meta.totalCols; }

  // This is needed for row-wise computation
  FFMatrixMeta getRowKey() {
      FFMatrixMeta rowMeta;
      rowMeta.blockRowIndex = meta.blockRowIndex;
      rowMeta.blockColIndex = 0;
      rowMeta.totalRows = meta.totalRows;
      rowMeta.totalCols = 1;
      return rowMeta;
  }

  // This is needed for row-wise computation
  FFMatrixData getRowSumValue() {
      FFMatrixData sumRowData;
      sumRowData.rowNums = data.rowNums;
      sumRowData.colNums = 1;
      int bufferLength = sumRowData.rowNums * sumRowData.colNums;
      sumRowData.rawData = pdb::makeObject<pdb::Vector<double>>(bufferLength, bufferLength);

      Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
          currentMatrix((data.rawData)->c_ptr(), data.rowNums, data.colNums);

      Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
          rowSumMatrix((sumRowData.rawData)->c_ptr(), sumRowData.rowNums, sumRowData.colNums);

      rowSumMatrix = currentMatrix.rowwise().sum();

      // std::cout <<"getRowSumValue Matrix :"<< std::endl;
      // this->print();
      // std::cout <<"Row wise Sum Matrix :"<< std::endl;
      // sumRowData.print();
      return sumRowData;
  }
};

#endif