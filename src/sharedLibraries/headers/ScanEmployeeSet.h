
#ifndef SCAN_EMPLOYEE_SET_H
#define SCAN_EMPLOYEE_SET_H

#include "ScanUserSet.h"
#include "SharedEmployee.h"

using namespace pdb;
class ScanEmployeeSet : public ScanUserSet<SharedEmployee> {

public:
    ENABLE_DEEP_COPY

    ScanEmployeeSet() {}

    ScanEmployeeSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
