
#ifndef SCAN_BUILTIN_EMPLOYEE_SET_H
#define SCAN_BUILTIN_EMPLOYEE_SET_H

#include "ScanUserSet.h"
#include "Employee.h"

using namespace pdb;
class ScanBuiltinEmployeeSet : public ScanUserSet<Employee> {

public:
    ENABLE_DEEP_COPY

    ScanBuiltinEmployeeSet() {}

    ScanBuiltinEmployeeSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
