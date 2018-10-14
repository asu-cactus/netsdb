
#ifndef SCAN_STRING_INT_PAIR_SET_H
#define SCAN_STRING_INT_PAIR_SET_H

#include "ScanUserSet.h"
#include "StringIntPair.h"

using namespace pdb;
class ScanStringIntPairSet : public ScanUserSet<StringIntPair> {

public:
    ENABLE_DEEP_COPY

    ScanStringIntPairSet() {}

    ScanStringIntPairSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
