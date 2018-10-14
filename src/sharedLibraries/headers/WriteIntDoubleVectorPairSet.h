#ifndef WRITE_INT_DOUBLE_VECTOR_PAIR_SET_H
#define WRITE_INT_DOUBLE_VECTOR_PAIR_SET_H

#include "WriteUserSet.h"
#include "IntDoubleVectorPair.h"

using namespace pdb;
class WriteIntDoubleVectorPairSet : public WriteUserSet<IntDoubleVectorPair> {

public:
    ENABLE_DEEP_COPY

    WriteIntDoubleVectorPairSet() {}

    WriteIntDoubleVectorPairSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
