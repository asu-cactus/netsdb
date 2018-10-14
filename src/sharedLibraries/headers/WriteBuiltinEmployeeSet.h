
#ifndef WRITE_BUILTIN_EMPLOYEE_SET_H
#define WRITE_BUILTIN_EMPLOYEE_SET_H

// by Jia, Mar 2017

#include "WriteUserSet.h"
#include "Employee.h"

using namespace pdb;
class WriteBuiltinEmployeeSet : public WriteUserSet<Employee> {

public:
    ENABLE_DEEP_COPY

    WriteBuiltinEmployeeSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    WriteBuiltinEmployeeSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
