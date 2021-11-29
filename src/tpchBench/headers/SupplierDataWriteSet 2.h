
#ifndef SUPPLIER_DATA_WRITE_SET_H
#define SUPPLIER_DATA_WRITE_SET_H

#include "WriteUserSet.h"
#include "SupplierData.h"

using namespace pdb;
class SupplierDataWriteSet : public WriteUserSet<SupplierData> {

public:
    ENABLE_DEEP_COPY

    SupplierDataWriteSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    SupplierDataWriteSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
