#ifndef TENSORBLOCK2D_H
#define TENSORBLOCK2D_H

#ifndef EIGEN_CODE
#define EIGEN_CODE 0
#endif

#include "TensorData2D.h"
#include "TensorMeta2D.h"
#include "Handle.h"
#include "PDBString.h"
#include "PDBVector.h"

// LA libraries:
#include <eigen3/Eigen/Dense>


namespace pdb {

template <class T>
class TensorBlock2D : public Object {

public:
  TensorData2D<T> data;
  pdb::Handle<TensorMeta2D> meta;
  bool partitionByCol = true;
  const static int librayCode = EIGEN_CODE;

  ENABLE_DEEP_COPY

  ~TensorBlock2D() {}

  TensorBlock2D() {}

  TensorBlock2D(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn) {
        meta = pdb::makeObject<TensorMeta2D>(blockRowIndexIn, blockColIndexIn);
        this->partitionByCol = partitionByCol;
      }

  TensorBlock2D(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols,  bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn) {
        meta = pdb::makeObject<TensorMeta2D>(blockRowIndexIn, blockColIndexIn, totalRows, totalCols);
        this->partitionByCol = partitionByCol;
      }


  TensorBlock2D(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols, int distinctBlockId, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn) {
        meta = pdb::makeObject<TensorMeta2D>(blockRowIndexIn, blockColIndexIn, totalRows, totalCols);
        this->partitionByCol = partitionByCol;
      }

  TensorBlock2D(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, pdb::Handle<pdb::Vector<T>> rawDataIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn, rawDataIn)  {
        meta = pdb::makeObject<TensorMeta2D>(blockRowIndexIn, blockColIndexIn);
        this->partitionByCol = partitionByCol;
      }

  TensorBlock2D(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols,
                pdb::Handle<pdb::Vector<T>> rawDataIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn, rawDataIn) {
        meta = pdb::makeObject<TensorMeta2D>(blockRowIndexIn, blockColIndexIn, totalRows, totalCols);
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

  pdb::Handle<pdb::Vector<T>> &getRawDataHandle() { return data.rawData; }

  pdb::Handle<TensorMeta2D> &getKey() { return meta; }

  TensorMeta2D &getKeyRef() { return *meta; }

  TensorData2D<T> &getValue() { return data; }

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
  pdb::Handle<TensorMeta2D> getRowKey() {
      pdb::Handle<TensorMeta2D> rowMeta = pdb::makeObject<TensorMeta2D>(meta->blockRowIndex, 0, meta->totalRows, 1);
      return rowMeta;
  }

  // This is needed for row-wise computation
  TensorData2D<T> getRowSumValue() {
      TensorData2D<T> sumRowData;
      sumRowData.rowNums = data.rowNums;
      sumRowData.colNums = 1;
      int bufferLength = sumRowData.rowNums * sumRowData.colNums;
      sumRowData.rawData = pdb::makeObject<pdb::Vector<T>>(bufferLength, bufferLength);

      Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
          currentMatrix((data.rawData)->c_ptr(), data.rowNums, data.colNums);

      Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
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

  // This is a overloaded + operator that will be invoked in the aggregation
  TensorBlock2D<T> &operator+(TensorBlock2D<T> &other) {
    /** //TODO
     * The current implementation only merges several FFMatrixBlock into a single matrix
     * so the output is ready to be sent to a UDF-based classifier. The ideal way is we 
     * can support concatenating the blocks along the feature's dimension. It can both 
     * x and y.  
     * */

    int totalRows = this->getTotalRowNums();
    int totalCols = this->getTotalColNums();
    int rowStart = other.getBlockRowIndex() * other.getRowNums();
    int colStart = other.getBlockColIndex() * other.getColNums();
    if ((other.getBlockRowIndex() < 0) || (rowStart >other.getTotalRowNums()) || (other.getRowNums() <=0)
		    || (other.getBlockColIndex() <0) || (colStart>other.getTotalColNums()) || (other.getColNums() <=0)) {
        return *this;
    }

    if ((this->getRowNums() == totalRows) && (this->getColNums() == totalCols)) {
    // the block (this) is already enlarged, the rest thing is adding the other block to 
    // its corresponding position
      for (int i = 0; i < other.getRowNums(); i++) {
        for (int j = 0; j < other.getColNums(); j++) {
          int finalRowIndex = rowStart + i;
          int finalColIndex = colStart + j;
          (*((this->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*(other.data.rawData))[i * other.data.colNums + j];
        }
      }
      return *this;
    } else {
      // need to create a large single block and add the blocks value to the created one
      pdb::Handle<TensorBlock2D<T>> result = pdb::makeObject<TensorBlock2D<T>>(0, 0, totalRows, totalCols, totalRows, totalCols);
      // Add block (this)
      for (int i = 0; i < this->getRowNums(); i++) {
        for (int j = 0; j < this->getColNums(); j++) {
          int finalRowIndex = rowStart + i;
          int finalColIndex = colStart + j;
          (*((result->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*(this->data.rawData))[i * this->data.colNums + j];
        }
      }
      // Add block (other)
      for (int i = 0; i < other.getRowNums(); i++) {
        for (int j = 0; j < other.getColNums(); j++) {
          int finalRowIndex = rowStart + i;
          int finalColIndex = colStart + j;
          (*((result->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*(other.data.rawData))[i * other.data.colNums + j];
        }
      }
      return *result;
    }
  }
};


}

#endif
