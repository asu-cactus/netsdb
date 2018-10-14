#ifndef WRITE_SUM_RESULT_SET_H
#define WRITE_SUM_RESULT_SET_H

#include "WriteUserSet.h"
#include "SumResult.h"

using namespace pdb;
class WriteSumResultSet : public WriteUserSet<SumResult> {

public:
    ENABLE_DEEP_COPY

    WriteSumResultSet() {}

    WriteSumResultSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
