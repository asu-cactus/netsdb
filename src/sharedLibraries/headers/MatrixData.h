#ifndef MATRIX_DATA_H
#define MATRIX_DATA_H

#define UNSETFLAG 0
#define MAXTRIXSUMFLAG 1
#define MATRIXROWMAXFLAG 2
#define MATRIXROWMINFLAG 3
#define MATRIXCOLMAXFLAG 4
#define MATRIXCOLMINFLAG 5


#include "Object.h"
#include "PDBVector.h"
#include "Handle.h"


class MatrixData : public pdb::Object {

private:
    void SumAggregate(MatrixData& other) {
        // std::cout << "Sum Aggregation +" << std::endl;
        for (int i = 0; i < rowNums * colNums; i++) {
            (*rawData)[i] += (*(other.rawData))[i];
        }
    }

    void RowMaxAggregate(MatrixData& other) {
        // std::cout << "Row-wise max Aggregation +" << std::endl;
        for (int i = 0; i < rowNums * colNums; i++) {
            if ((*rawData)[i] < (*(other.rawData))[i]) {
                (*rawData)[i] = (*(other.rawData))[i];
            }
        }
    }

    void RowMinAggregate(MatrixData& other) {
        // std::cout << "Row-wise min Aggregation +" << std::endl;
        for (int i = 0; i < rowNums * colNums; i++) {
            if ((*rawData)[i] > (*(other.rawData))[i]) {
                (*rawData)[i] = (*(other.rawData))[i];
            }
        }
    }

    void ColMaxAggregate(MatrixData& other) {
        // std::cout << "Col-wise max Aggregation +" << std::endl;
        for (int i = 0; i < rowNums * colNums; i++) {
            if ((*rawData)[i] < (*(other.rawData))[i]) {
                (*rawData)[i] = (*(other.rawData))[i];
            }
        }
    }

    void ColMinAggregate(MatrixData& other) {
        // std::cout << "Col-wise min Aggregation +" << std::endl;
        for (int i = 0; i < rowNums * colNums; i++) {
            if ((*rawData)[i] > (*(other.rawData))[i]) {
                (*rawData)[i] = (*(other.rawData))[i];
            }
        }
    }

public:
    ENABLE_DEEP_COPY

    ~MatrixData() {}
    MatrixData() {}


    pdb::Handle<pdb::Vector<double>> rawData;
    int rowNums = 0;
    int colNums = 0;

    int flag = UNSETFLAG;

    void setSumFlag() {
        flag = MAXTRIXSUMFLAG;
    }

    void setRowMaxFlag() {
        flag = MATRIXROWMAXFLAG;
    }

    void setRowMinFlag() {
        flag = MATRIXROWMINFLAG;
    }

    void setColMaxFlag() {
        flag = MATRIXCOLMAXFLAG;
    }

    void setColMinFlag() {
        flag = MATRIXCOLMINFLAG;
    }


    void print() {
        std::cout << "Flag: " << flag << " Row: " << rowNums << " Col: " << colNums
                  << " Buffer size:" << rawData->size() << " ";
        if (rawData->size() != rowNums * colNums) {
            std::cout << "Matrix Error: size misMatch!" << std::endl;
        } else {
            for (int i = 0; i < rawData->size(); i++) {
                if (i % colNums == 0) {
                    std::cout << std::endl;
                }
                std::cout << (*rawData)[i] << " ";
            }
            std::cout << std::endl;
        }
    }


    MatrixData& operator+(MatrixData& other) {
        // std::cout<< "+ operator:" << std::endl;
        // this->print();
        // other.print();
        if (rowNums != other.rowNums || colNums != other.colNums) {
            this->rawData->clear();
        } else {
            if (flag == MAXTRIXSUMFLAG) {
                SumAggregate(other);
            } else if (flag == MATRIXROWMAXFLAG) {
                RowMaxAggregate(other);
            } else if (flag == MATRIXROWMINFLAG) {
                RowMinAggregate(other);
            } else if (flag == MATRIXCOLMAXFLAG) {
                ColMaxAggregate(other);
            } else if (flag == MATRIXCOLMINFLAG) {
                ColMinAggregate(other);
            } else {
                std::cerr << "Operator not supported yet!" << std::endl;
            }
        }
        return *this;
    }

    size_t hash () const { return 0; }
};

#endif
