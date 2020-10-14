#ifndef FF_MATRIXBLOCK_H
#define FF_MATRIXBLOCK_H

#include "FFMatrixData.h"
#include "MatrixBlock.h"

class FFMatrixBlock : public MatrixBlock {
private:
  FFMatrixData data;

public:
  ENABLE_DEEP_COPY

  ~FFMatrixBlock() {}
  FFMatrixBlock() {}

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn) {
    getKey().blockRowIndex = blockRowIndexIn;
    getKey().blockColIndex = blockColIndexIn;
    // getKey().rowNums = rowNumsIn;
    // getKey().colNums = colNumsIn;
    data.rowNums = rowNumsIn;
    data.colNums = colNumsIn;
    data.rawData = pdb::makeObject<pdb::Vector<double>>(rowNumsIn * colNumsIn,
                                                        rowNumsIn * colNumsIn);
    // std::cout << "This is risky, please call the other constructor.
    // FFMatrixBlock constructor RawData size:" << (data.rawData)->size() <<
    // std::endl;
  }

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols) {
    getKey().blockRowIndex = blockRowIndexIn;
    getKey().blockColIndex = blockColIndexIn;
    getKey().totalRows = totalRows;
    getKey().totalCols = totalCols;
    data.rowNums = rowNumsIn;
    data.colNums = colNumsIn;
    data.rawData = pdb::makeObject<pdb::Vector<double>>(rowNumsIn * colNumsIn,
                                                        rowNumsIn * colNumsIn);
    // std::cout << "FFMatrixBlock constructor RawData size:" <<
    // (data.rawData)->size() << std::endl;
  }

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, pdb::Handle<pdb::Vector<double>> rawDataIn) {
    getKey().blockRowIndex = blockRowIndexIn;
    getKey().blockColIndex = blockColIndexIn;
    // getKey().rowNums = rowNumsIn;
    // getKey().colNums = colNumsIn;
    data.rowNums = rowNumsIn;
    data.colNums = colNumsIn;
    data.rawData = rawDataIn;
    // std::cout << "This is risky, please call the other constructor.
    // FFMatrixBlock constructor RawData size:" << (data.rawData)->size() <<
    // std::endl;
  }

  FFMatrixBlock(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn,
                int colNumsIn, int totalRows, int totalCols,
                pdb::Handle<pdb::Vector<double>> rawDataIn) {
    getKey().blockRowIndex = blockRowIndexIn;
    getKey().blockColIndex = blockColIndexIn;
    getKey().totalRows = totalRows;
    getKey().totalCols = totalCols;
    data.rowNums = rowNumsIn;
    data.colNums = colNumsIn;
    data.rawData = rawDataIn;
    // std::cout << "FFMatrixBlock constructor RawData size:" <<
    // (data.rawData)->size() << std::endl;
  }

    int getRowNums() {
        return data.rowNums;
    }

    int getColNums() {
        return data.colNums;
    }

    pdb::Handle<pdb::Vector<double>>& getRawDataHandle() {
        return data.rawData;
    }

    MatrixMeta& getKey() {
        return MatrixBlock::getKey();
    }

    FFMatrixData& getValue() {
        return data;
    }

    FFMatrixData& getMultiplyValue() {
        data.setSumFlag();
        return data;
    }
};

#endif