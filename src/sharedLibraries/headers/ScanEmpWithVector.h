
#ifndef SCAN_EMP_WITH_VECTOR_H
#define SCAN_EMP_WITH_VECTOR_H

#include "ScanUserSet.h"
#include "EmpWithVector.h"

using namespace pdb;
class ScanEmpWithVector : public ScanUserSet<EmpWithVector> {

public:
    ENABLE_DEEP_COPY

    ScanEmpWithVector() {}

    ScanEmpWithVector(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
