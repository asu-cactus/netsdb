
#ifndef WRITE_INT_SET_H
#define WRITE_INT_SET_H

// by Jia, Mar 2017

#include "WriteUserSet.h"

using namespace pdb;
class WriteIntSet : public WriteUserSet<int> {

public:
    ENABLE_DEEP_COPY

    WriteIntSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    WriteIntSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
