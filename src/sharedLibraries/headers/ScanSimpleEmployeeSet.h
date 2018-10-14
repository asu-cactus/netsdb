
#ifndef SCAN_SIMPLE_EMPLOYEE_SET_H
#define SCAN_SIMPLE_EMPLOYEE_SET_H

#include "ScanUserSet.h"
#include "SimpleEmployee.h"

using namespace pdb;
class ScanSimpleEmployeeSet : public ScanUserSet<SimpleEmployee> {

public:
    ENABLE_DEEP_COPY

    ScanSimpleEmployeeSet() {}

    ScanSimpleEmployeeSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
