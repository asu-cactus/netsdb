
#ifndef WRITE_EMPLOYEE_SET_H
#define WRITE_EMPLOYEE_SET_H

// by Binhang, May 2017

#include "WriteUserSet.h"
#include "SharedEmployee.h"

using namespace pdb;
class WriteEmployeeSet : public WriteUserSet<SharedEmployee> {

public:
    ENABLE_DEEP_COPY

    WriteEmployeeSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    WriteEmployeeSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif