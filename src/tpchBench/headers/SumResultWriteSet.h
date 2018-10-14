
#ifndef SUM_RESULT_WRITE_SET_H
#define SUM_RESULT_WRITE_SET_H

#include "WriteUserSet.h"
#include "Customer.h"
#include "SumResult.h"

using namespace pdb;
class SumResultWriteSet : public WriteUserSet<SumResult> {

public:
    ENABLE_DEEP_COPY

    SumResultWriteSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    SumResultWriteSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
