
#ifndef SCAN_CUSTOMER_SET_H
#define SCAN_CUSTOMER_SET_H

#include "ScanUserSet.h"
#include "Customer.h"

using namespace pdb;
class ScanCustomerSet : public ScanUserSet<Customer> {

public:
    ENABLE_DEEP_COPY

    ScanCustomerSet() {}

    ScanCustomerSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};
#endif
