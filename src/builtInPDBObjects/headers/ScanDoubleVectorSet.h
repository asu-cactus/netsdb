
#ifndef SCAN_DOUBLE_VECTOR_SET_H
#define SCAN_DOUBLE_VECTOR_SET_H

#include "ScanUserSet.h"
#include "DoubleVector.h"

// PRELOAD %ScanDoubleVectorSet%

using namespace pdb;
class ScanDoubleVectorSet : public ScanUserSet<DoubleVector> {

public:
    ENABLE_DEEP_COPY

    ScanDoubleVectorSet() {}

    ScanDoubleVectorSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
