
#ifndef SUPPLIER_INFO_WRITE_SET_H
#define SUPPLIER_INFO_WRITE_SET_H

#include "WriteUserSet.h"
#include "SupplierInfo.h"

using namespace pdb;
class SupplierInfoWriteSet : public WriteUserSet<SupplierInfo> {

public:
    ENABLE_DEEP_COPY

    SupplierInfoWriteSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    SupplierInfoWriteSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
