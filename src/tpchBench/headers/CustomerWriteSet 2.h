
#ifndef CUSTOMER_WRITE_SET_H
#define CUSTOMER_WRITE_SET_H

#include "WriteUserSet.h"
#include "Customer.h"

using namespace pdb;
class CustomerWriteSet : public WriteUserSet<Customer> {

public:
    ENABLE_DEEP_COPY

    CustomerWriteSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    CustomerWriteSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
