#ifndef FF_MATRIXBLOCK_SCANNER_H
#define FF_MATRIXBLOCK_SCANNER_H

#include "ScanUserSet.h"
//#include "BuiltInMatrixBlock.h"
#include "FFMatrixBlock.h"

using namespace pdb;
class FFMatrixBlockScanner : public ScanUserSet<FFMatrixBlock> {

public:
    ENABLE_DEEP_COPY

    FFMatrixBlockScanner() {}

    FFMatrixBlockScanner(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif