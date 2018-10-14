#ifndef SCAN_DOUBLE_ARRAY_SET_H
#define SCAN_DOUBLE_ARRAY_SET_H

#include "ScanUserSet.h"

#ifndef NUM_KMEANS_DIMENSIONS
#define NUM_KMEANS_DIMENSIONS 1000
#endif


using namespace pdb;
class ScanDoubleArraySet : public ScanUserSet<double[NUM_KMEANS_DIMENSIONS]> {

public:
    ENABLE_DEEP_COPY

    ScanDoubleArraySet () {}

    ScanDoubleArraySet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }


};


#endif
