
#ifndef LA_SCAN_MATRIXBLOCK_SET_H
#define LA_SCAN_MATRIXBLOCK_SET_H

#include "ScanUserSet.h"
//#include "BuiltInMatrixBlock.h"
#include "MatrixBlock.h"

// By Binhang, May 2017

using namespace pdb;
class LAScanMatrixBlockSet : public ScanUserSet<MatrixBlock> {

public:
    ENABLE_DEEP_COPY

    LAScanMatrixBlockSet() {}

    LAScanMatrixBlockSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif