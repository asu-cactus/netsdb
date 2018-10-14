
#ifndef SCAN_KMEANS_DOUBLE_VECTOR_SET_H
#define SCAN_KMEANS_DOUBLE_VECTOR_SET_H

#include "ScanUserSet.h"
#include "KMeansDoubleVector.h"


using namespace pdb;
class ScanKMeansDoubleVectorSet : public ScanUserSet<KMeansDoubleVector> {

public:
    ENABLE_DEEP_COPY

    ScanKMeansDoubleVectorSet() {}

    ScanKMeansDoubleVectorSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
