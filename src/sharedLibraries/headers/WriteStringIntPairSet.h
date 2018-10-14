
#ifndef WRITE_STRING_INT_PAIR_SET_H
#define WRITE_STRING_INT_PAIR_SET_H

// by Jia, Mar 2017

#include "WriteUserSet.h"
#include "StringIntPair.h"

using namespace pdb;
class WriteStringIntPairSet : public WriteUserSet<StringIntPair> {

public:
    ENABLE_DEEP_COPY

    WriteStringIntPairSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    WriteStringIntPairSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
