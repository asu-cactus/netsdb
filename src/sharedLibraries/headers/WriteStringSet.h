
#ifndef WRITE_STRING_SET_H
#define WRITE_STRING_SET_H

// by Jia, Mar 2017

#include "WriteUserSet.h"
#include "PDBString.h"

using namespace pdb;
class WriteStringSet : public WriteUserSet<String> {

public:
    ENABLE_DEEP_COPY

    WriteStringSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    WriteStringSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
