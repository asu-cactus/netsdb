#ifndef SCAN_STARSIN_SET_H
#define SCAN_STARSIN_SET_H

#include "ScanUserSet.h"
#include "StarsIn.h"

using namespace pdb;
class ScanStarsInSet : public ScanUserSet<StarsIn> {

public:
    ENABLE_DEEP_COPY

    ScanStarsInSet() {}

    ScanStarsInSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};

#endif