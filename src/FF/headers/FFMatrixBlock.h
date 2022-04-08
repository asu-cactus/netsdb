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
public:
  FFMatrixData data;
  pdb::Handle<FFMatrixMeta> meta;
  bool partitionByCol = true;
  const static int librayCode = EIGEN_CODE;

  ENABLE_DEEP_COPY

  ~FFMatrixBlock() {}

  FFMatrixBlock() {}

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn) {
        meta = pdb::makeObject<FFMatrixMeta>(blockRowIndexIn, blockColIndexIn);
        this->partitionByCol = partitionByCol;
      }

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols,  bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn) {
        meta = pdb::makeObject<FFMatrixMeta>(blockRowIndexIn, blockColIndexIn, totalRows, totalCols);
        this->partitionByCol = partitionByCol;
      }

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, pdb::Handle<pdb::Vector<double>> rawDataIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn, rawDataIn)  {
        meta = pdb::makeObject<FFMatrixMeta>(blockRowIndexIn, blockColIndexIn);
        this->partitionByCol = partitionByCol;
      }

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols,
                pdb::Handle<pdb::Vector<double>> rawDataIn, bool partitionByCol = true)
      : data(rowNumsIn, colNumsIn, rawDataIn) {
        meta = pdb::makeObject<FFMatrixMeta>(blockRowIndexIn, blockColIndexIn, totalRows, totalCols);
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
    int numYBlocks = ceil(getTotalColNums() / (double)getColNums());
    return getBlockColIndex() == numYBlocks - 1;
  }

  bool isLastRowBlock() {
    int numXBlocks = ceil(getTotalRowNums() / (double)getRowNums());
    return getBlockRowIndex() == numXBlocks - 1;
  }

  int getColNums() { return data.colNums; }

  pdb::Handle<pdb::Vector<double>> &getRawDataHandle() { return data.rawData; }

  pdb::Handle<FFMatrixMeta> &getKey() { return meta; }

  FFMatrixMeta &getKeyRef() { return *meta; }

  FFMatrixData &getValue() { return data; }

  int getBlockRowIndex() { return meta->blockRowIndex; }

  int getBlockColIndex() { return meta->blockColIndex; }

  int getTotalRowNums() { return meta->totalRows; }

  int getTotalColNums() { return meta->totalCols; }

  int getNumRowBlocks() {
      return ceil(getTotalRowNums() / (double)getRowNums());
  }

  int getNumColBlocks() {
      return ceil(getTotalColNums() / (double)getColNums());
  }

  // This is needed for row-wise computation
  pdb::Handle<FFMatrixMeta> getRowKey() {
      pdb::Handle<FFMatrixMeta> rowMeta = pdb::makeObject<FFMatrixMeta>(meta->blockRowIndex, 0, meta->totalRows, 1);
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
  FFMatrixBlock &operator+(FFMatrixBlock &other) {
    /** //TODO
     * The current implementation only merges several FFMatrixBlock into a single matrix
     * so the output is ready to be sent to a UDF-based classifier. The ideal way is we 
     * can support concatenating the blocks along the feature's dimension. It can both 
     * x and y.  
     * */

    /**
     * Following code is used for debug, keep it here temporary  
    std::cout << "entering the overloaded + " << this->hash() << std::endl;
    std::cout << "block 1 INFO: " << std::endl;
    std::cout << " inNumRow: " << this->getRowNums() << " inNumCol: " << this->getColNums() << " inBlockRowIndex: " << 
                    this->getBlockRowIndex() << " inBlockColIndex: " << this->getBlockColIndex() << 
                    " inTotalRowNums: " << this->getTotalRowNums() << " inTotalColNums: " << this->getTotalColNums() <<   std::endl;
    std::cout << "block 2 INFO: " << std::endl;
    std::cout << " inNumRow: " << other.getRowNums() << " inNumCol: " << other.getColNums() << " inBlockRowIndex: " << 
                    other.getBlockRowIndex() << " inBlockColIndex: " << other.getBlockColIndex() << 
                    " inTotalRowNums: " << other.getTotalRowNums() << " inTotalColNums: " << other.getTotalColNums() <<   std::endl;
    **/

    int totalRows = this->getTotalRowNums();
    int totalCols = this->getTotalColNums();

    if ((this->getRowNums() == totalRows) && (this->getColNums() == totalCols)) {
    // the block (this) is already enlarged, the rest thing is adding the other block to 
    // its corresponding position
      for (int i = 0; i < other.getRowNums(); i++) {
        for (int j = 0; j < other.getColNums(); j++) {
          int finalRowIndex = other.getBlockRowIndex() * other.getRowNums() + i;
          int finalColIndex = other.getBlockColIndex() * other.getColNums() + j;
          (*((this->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*(other.data.rawData))[i * other.data.colNums + j];
        }
      }
      return *this;
    } else {
      // need to create a large single block and add the blocks value to the created one
      pdb::Handle<FFMatrixBlock> result = pdb::makeObject<FFMatrixBlock>(0, 0, totalRows, totalCols, totalRows, totalCols);
      // Add block (this)
      for (int i = 0; i < this->getRowNums(); i++) {
        for (int j = 0; j < this->getColNums(); j++) {
          int finalRowIndex = this->getBlockRowIndex() * this->getRowNums() + i;
          int finalColIndex = this->getBlockColIndex() * this->getColNums() + j;
          (*((result->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*(this->data.rawData))[i * this->data.colNums + j];
        }
      }
      // Add block (other)
      for (int i = 0; i < other.getRowNums(); i++) {
        for (int j = 0; j < other.getColNums(); j++) {
          int finalRowIndex = other.getBlockRowIndex() * other.getRowNums() + i;
          int finalColIndex = other.getBlockColIndex() * other.getColNums() + j;
          (*((result->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*(other.data.rawData))[i * other.data.colNums + j];
        }
      }
      return *result;
    }
  }
};

#endif
