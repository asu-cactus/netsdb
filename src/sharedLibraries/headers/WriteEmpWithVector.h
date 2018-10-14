
#ifndef WRITE_EMP_WITH_VECTOR_H
#define WRITE_EMP_WITH_VECTOR_H

#include "WriteUserSet.h"
#include "TopKQueue.h"
#include "EmpWithVector.h"

using namespace pdb;
class WriteEmpWithVector : public WriteUserSet<TopKQueue<double, Handle<EmpWithVector>>> {

public:
    ENABLE_DEEP_COPY

    WriteEmpWithVector() {}

    WriteEmpWithVector(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
