
#ifndef WRITE_DOUBLE_SET_H
#define WRITE_DOUBLE_SET_H

// by Jia, May 2017

#include "WriteUserSet.h"

using namespace pdb;
class WriteDoubleSet : public WriteUserSet<double> {

public:
    ENABLE_DEEP_COPY

    WriteDoubleSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    WriteDoubleSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
