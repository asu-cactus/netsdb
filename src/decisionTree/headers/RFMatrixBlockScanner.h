#ifndef RF_MATRIXBLOCK_SCANNER_H
#define RF_MATRIXBLOCK_SCANNER_H

#include "ScanUserSet.h"
#include "RFMatrixBlock.h"

using namespace pdb;

class RFMatrixBlockScanner : public ScanUserSet<RFMatrixBlock> {

public:
    ENABLE_DEEP_COPY

    RFMatrixBlockScanner() {}

    RFMatrixBlockScanner(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};

#endif