#ifndef CUSTOMER_SUPPLIER_PART_FLAT_H
#define CUSTOMER_SUPPLIER_PART_FLAT_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"

// This class represents a triple that holds a triple of (customerName, SupplierName,
// vector<PartID>)

class CustomerSupplierPartFlat : public pdb::Object {

public:
    pdb::String customerName;
    pdb::String supplierName;
    int partKey;

    ENABLE_DEEP_COPY

    // Default constructor:
    CustomerSupplierPartFlat() {}

    // Default destructor:
    ~CustomerSupplierPartFlat() {}

    // Constructor with arguments:
    CustomerSupplierPartFlat(pdb::String customerName, pdb::String supplierName, int partKey) {
        this->customerName = customerName;
        this->supplierName = supplierName;
        this->partKey = partKey;
    }

    pdb::String& getCustomerName() {
        return customerName;
    }

    void setCustomerName(pdb::String customerName) {
        this->customerName = customerName;
    }

    pdb::String& getSupplierName() {
        return supplierName;
    }

    void setSupplierName(pdb::String supplierName) {
        this->supplierName = supplierName;
    }

    int getPartKey() {
        return partKey;
    }

    void setPartKey(int partKey) {
        this->partKey = partKey;
    }
};

#endif
