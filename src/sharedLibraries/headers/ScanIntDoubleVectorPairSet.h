#ifndef SCAN_INT_DOUBLE_VECTOR_PAIR_SET_H
#define SCAN_INT_DOUBLE_VECTOR_PAIR_SET_H

#include "ScanUserSet.h"
#include "IntDoubleVectorPair.h"

using namespace pdb;
class ScanIntDoubleVectorPairSet : public ScanUserSet<IntDoubleVectorPair> {

public:
    ENABLE_DEEP_COPY

    ScanIntDoubleVectorPairSet() {}

    ScanIntDoubleVectorPairSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};

#endif
