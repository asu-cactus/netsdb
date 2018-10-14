
#ifndef SCAN_INT_SET_H
#define SCAN_INT_SET_H

#include "ScanUserSet.h"

using namespace pdb;
class ScanIntSet : public ScanUserSet<int> {

public:
    ENABLE_DEEP_COPY

    ScanIntSet() {}

    ScanIntSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
