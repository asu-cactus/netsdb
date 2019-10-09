#if !defined(MATRIX_BLOCK_H) && !defined(BUILTIN_MATRIX_BLOCK_H)
#define MATRIX_BLOCK_H

//#ifndef MAX_BLOCK_SIZE
//    #define MAX_BLOCK_SIZE 10000
//#endif

#ifndef EIGEN_CODE
#define EIGEN_CODE 0
#endif

// by Binhang, May 2017
#include <mkl.h>
#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include "ExportableObject.h"
#include "StringIntPair.h"
#include <vector>

// LA libraries:
#include <eigen3/Eigen/Dense>


#include "MatrixMeta.h"
#include "MatrixData.h"
#include "LAMaxElementValueType.h"
#include "LAMinElementValueType.h"


class MatrixSparse : public ExportableObject {

private:
    MatrixMeta meta;
    MatrixData data;


public:
    const static int librayCode = EIGEN_CODE;

    ENABLE_DEEP_COPY

    ~MatrixSparse() {}
    MatrixSparse() {}


        

   MatrixSparse(int blockRowIndexIn, int blockColIndexIn, int rowNumsIn, int colNumsIn){
        meta.blockRowIndex = blockRowIndexIn;
        meta.blockColIndex = blockColIndexIn;
        // meta.rowNums = rowNumsIn;
        // meta.colNums = colNumsIn;
        data.rowNums = rowNumsIn;
        data.colNums = colNumsIn;
        data.rawData =
            pdb::makeObject<pdb::Vector<double>>(rowNumsIn * colNumsIn, rowNumsIn * colNumsIn);
        // std::cout << "This is risky, please call the other constructor. MatrixBlock constructor
        // RawData size:" << (data.rawData)->size() << std::endl;
    }

    MatrixSparse(int blockRowIndexIn,
                int blockColIndexIn,
                int rowNumsIn,
                int colNumsIn,
                int totalRows,
                int totalCols){
        meta.blockRowIndex = blockRowIndexIn;
        meta.blockColIndex = blockColIndexIn;
        meta.totalRows = totalRows;
        meta.totalCols = totalCols;
        data.rowNums = rowNumsIn;
        data.colNums = colNumsIn;
       // std::vector<float> values;
	//std::vector<double> valuesD(values.begin(),values.end());
	data.rawData = pdb::makeObject<pdb::Vector<double>>(rowNumsIn * colNumsIn, rowNumsIn * colNumsIn);
        // std::cout << "MatrixBlock constructor RawData size:" << (data.rawData)->size() <<
        // std::endl;
    }


    size_t hash() const {
        return 10000*meta.blockRowIndex + meta.blockColIndex;
    }

    const std::string toString() const {
        return std::string("Block: (")+std::to_string(meta.blockRowIndex)+std::string(",")
               +std::to_string(meta.blockColIndex)+std::string(")");
    }

    void printMeta() {
        std::cout << "Block: (" << meta.blockRowIndex << "," << meta.blockColIndex << "), size: ("
                  << data.rowNums << "," << data.colNums << "), length:" << data.rawData->size()
                  << " ";
    }

    void print() override {
        std::cout << "Block: (" << meta.blockRowIndex << "," << meta.blockColIndex << "), size: ("
                  << data.rowNums << "," << data.colNums << "), length:" << data.rawData->size()
                  << " ";
        if (data.rawData->size() != data.rowNums * data.colNums) {
            std::cout << "Matrix Error: size misMatch!" << std::endl;
        } else {
            for (int i = 0; i < data.rawData->size(); i++) {
                if (i % data.colNums == 0) {
                    std::cout << std::endl;
                }
                std::cout << (*(data.rawData))[i] << " ";
            }
            std::cout << std::endl;
        }
    }

    friend std::ostream& operator<<(std::ostream& stream, const StringIntPair& printMe) {
        return stream << printMe.toString();
    }

    std::string toSchemaString(std::string format) override {
        return "";
    }

    std::string toValueString(std::string format) override {
        return "";
    }

    std::vector<std::string> getSupportedFormats() override {
        std::vector<std::string> ret;
        return ret;
    }

    int getBlockRowIndex() {
        return meta.blockRowIndex;
    }

    int getBlockColIndex() {
        return meta.blockColIndex;
    }

    int getRowNums() {
        return data.rowNums;
    }

    int getColNums() {
        return data.colNums;
    }

    int getTotalRowNums() {
        return meta.totalRows;
    }

    int getTotalColNums() {
        return meta.totalCols;
    }
  int blockRowIndexIn =0;
  int blockColIndexIn =0;
  MKL_INT rowNumsIn =0;
   MKL_INT colNumsIn =0;


   pdb::Vector<MKL_INT> rowIndices;

     pdb::Vector<MKL_INT> colIndices;
     
   pdb::Vector<float> values;
  
    pdb::Handle<pdb::Vector<double>>& getRawDataHandle() {
        return data.rawData;
    }

    MatrixMeta& getKey() {
        return meta;
    }

    MatrixData& getValue() {
        return data;
    }


    MatrixMeta& getMultiplyKey() {
        return meta;
    }

    MatrixData& getMultiplyValue() {
        data.setSumFlag();
        return data;
    }

    // This is needed for row-wise computation
    MatrixMeta getRowKey() {
        MatrixMeta rowMeta;
        rowMeta.blockRowIndex = meta.blockRowIndex;
        rowMeta.blockColIndex = 0;
        return rowMeta;
    }

    // This is needed for col-wise computation
    MatrixMeta getColKey() {
        MatrixMeta colMeta;
        colMeta.blockRowIndex = 0;
        colMeta.blockColIndex = meta.blockColIndex;
        return colMeta;
    }



    // inverse
    MatrixSparse& operator+(MatrixSparse& other) {
        int totalRows = (this->meta).totalRows;
        int totalCols = (this->meta).totalCols;
        // This is already a merged block
        if ((this->data).colNums == (this->meta).totalCols &&
            (this->data).rowNums == (this->meta).totalRows) {
            for (int i = 0; i < other.data.rowNums; i++) {
                for (int j = 0; j < other.data.colNums; j++) {
                    int finalRowIndex = other.meta.blockRowIndex * other.data.rowNums + i;
                    int finalColIndex = other.meta.blockColIndex * other.data.colNums + j;
                    (*((this->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*(other.data.rawData))[i * other.data.colNums + j];
                }
            }
            // this->print();
            return *this;
        }
        // Otherwise, should be called only once.
        else {
            Handle<MatrixSparse> result =
                makeObject<MatrixSparse>(0, 0, totalRows, totalCols, totalRows, totalCols);
            for (int i = 0; i < (this->data).rowNums; i++) {
                for (int j = 0; j < (this->data).colNums; j++) {
                    int finalRowIndex = (this->meta).blockRowIndex * (this->data).rowNums + i;
                    int finalColIndex = (this->meta).blockColIndex * (this->data).colNums + j;
                    (*((result->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*((this->data).rawData))[i * (this->data).colNums + j];
                }
            }
            for (int i = 0; i < other.data.rowNums; i++) {
                for (int j = 0; j < other.data.colNums; j++) {
                    int finalRowIndex = other.meta.blockRowIndex * other.data.rowNums + i;
                    int finalColIndex = other.meta.blockColIndex * other.data.colNums + j;
                    (*((result->data).rawData))[finalRowIndex * totalCols + finalColIndex] =
                        (*(other.data.rawData))[i * other.data.colNums + j];
                }
            }
            // result->print();
            return *result;
        }
    }



};


#endif
