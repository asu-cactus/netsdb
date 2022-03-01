#ifndef RF_MATRIXBLOCK_H
#define RF_MATRIXBLOCK_H

#ifndef EIGEN_CODE
#define EIGEN_CODE 0
#endif

#include "RFMatrixData.h"
#include "RFMatrixMeta.h"
#include "Handle.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "StringIntPair.h"

// LA libraries:
#include <eigen3/Eigen/Dense>

class RFMatrixBlock : public pdb::Object {
public:
  RFMatrixData data;
  pdb::Handle<RFMatrixMeta> meta;
  bool partitionByCol = true;
  const static int librayCode = EIGEN_CODE;

  ENABLE_DEEP_COPY

  ~RFMatrixBlock() {}

  RFMatrixBlock() {}

  RFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn) {
        meta = pdb::makeObject<RFMatrixMeta>(blockRowIndexIn, blockColIndexIn);
        this->partitionByCol = partitionByCol;
      }

  RFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols,  bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn) {
        meta = pdb::makeObject<RFMatrixMeta>(blockRowIndexIn, blockColIndexIn, totalRows, totalCols);
        this->partitionByCol = partitionByCol;
      }

  RFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, pdb::Handle<pdb::Vector<float>> rawDataIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn, rawDataIn)  {
        meta = pdb::makeObject<RFMatrixMeta>(blockRowIndexIn, blockColIndexIn);
        this->partitionByCol = partitionByCol;
      }

  RFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols,
                pdb::Handle<pdb::Vector<float>> rawDataIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn, rawDataIn) {
        meta = pdb::makeObject<RFMatrixMeta>(blockRowIndexIn, blockColIndexIn, totalRows, totalCols);
        this->partitionByCol = partitionByCol;
      }

  int getRowNums() { return data.rowNums; }

  int rowIndexStart() {
    return getBlockRowIndex() * getRowNums();
  }

  int rowIndexEnd() {
    return getBlockRowIndex() * getRowNums() + getRowNums();
  }

  bool isLastColBlock() {
    int numYBlocks = ceil(getTotalColNums() / (float)getColNums());
    return getBlockColIndex() == numYBlocks - 1;
  }

  bool isLastRowBlock() {
    int numXBlocks = ceil(getTotalRowNums() / (float)getRowNums());
    return getBlockRowIndex() == numXBlocks - 1;
  }

  int getColNums() { return data.colNums; }

  pdb::Handle<pdb::Vector<float>> &getRawDataHandle() { return data.rawData; }

  pdb::Handle<RFMatrixMeta> &getKey() { return meta; }

  RFMatrixMeta &getKeyRef() { return *meta; }

  RFMatrixData &getValue() { return data; }

  int getBlockRowIndex() { return meta->blockRowIndex; }

  int getBlockColIndex() { return meta->blockColIndex; }

  int getTotalRowNums() { return meta->totalRows; }

  int getTotalColNums() { return meta->totalCols; }

  int getNumRowBlocks() {
      return ceil(getTotalRowNums() / (float)getRowNums());
  }

  int getNumColBlocks() {
      return ceil(getTotalColNums() / (float)getColNums());
  }

  // This is needed for row-wise computation
  pdb::Handle<RFMatrixMeta> getRowKey() {
      pdb::Handle<RFMatrixMeta> rowMeta = pdb::makeObject<RFMatrixMeta>(meta->blockRowIndex, 0, meta->totalRows, 1);
      return rowMeta;
  }

  // This is needed for row-wise computation
  RFMatrixData getRowSumValue() {
      RFMatrixData sumRowData;
      sumRowData.rowNums = data.rowNums;
      sumRowData.colNums = 1;
      int bufferLength = sumRowData.rowNums * sumRowData.colNums;
      sumRowData.rawData = pdb::makeObject<pdb::Vector<float>>(bufferLength, bufferLength);

      Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
          currentMatrix((data.rawData)->c_ptr(), data.rowNums, data.colNums);

      Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
          rowSumMatrix((sumRowData.rawData)->c_ptr(), sumRowData.rowNums, sumRowData.colNums);

      rowSumMatrix = currentMatrix.rowwise().sum();
      return sumRowData;
  }

  void print() {
     std::cout << "BlockRowIndex:" << getBlockRowIndex() << std::endl;
     std::cout << "BlockColumnIndex:" << getBlockColIndex() << std::endl;
     std::cout << "TotalRowNums:" << getTotalRowNums() << std::endl;
     std::cout << "TotalColNums:" << getTotalColNums() << std::endl;
  }  

  size_t hash() const override{
     if (partitionByCol)
         return pdb::Hasher<int>::hash(meta->blockColIndex);  
     else
         return pdb::Hasher<int>::hash(meta->blockRowIndex);        
  }

};

#endif