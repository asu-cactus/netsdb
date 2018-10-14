
#ifndef WRITE_DOUBLE_Vector_SET_H
#define WRITE_DOUBLE_Vector_SET_H

// by Shangyu, May 2017

#include "WriteUserSet.h"
#include "DoubleVector.h"

using namespace pdb;
class WriteDoubleVectorSet : public WriteUserSet<DoubleVector> {

public:
    ENABLE_DEEP_COPY

    WriteDoubleVectorSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    WriteDoubleVectorSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
