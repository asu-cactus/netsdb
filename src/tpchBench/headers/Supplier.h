#ifndef SUPPLIER_H
#define SUPPLIER_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include <vector>

// This class represents a Supplier Object according to the TPC-H Database Benchmark

// CREATE TABLE [dbo].[SUPPLIER](
// [S_SUPPKEY] [int] NOT NULL,
// [S_NAME] [char](25) NOT NULL,
// [S_ADDRESS] [varchar](40) NOT NULL,
// [S_NATIONKEY] [int] NOT NULL,
// [S_PHONE] [char](15) NOT NULL,
// [S_ACCTBAL] [decimal](15, 2) NOT NULL,
// [S_COMMENT] [varchar](101) NOT NULL
// );

class Supplier : public pdb::Object {

public:
    int supplierKey;
    pdb::String name;
    pdb::String address;
    int nationKey;
    pdb::String phone;
    double accbal;
    pdb::String comment;


    ENABLE_DEEP_COPY

    ~Supplier() {}

    Supplier() {}

    // Constructor with arguments:
    Supplier(int supplierKey,
             std::string name,
             std::string address,
             int nationKey,
             std::string phone,
             double accbal,
             std::string comment) {
        this->supplierKey = supplierKey;
        this->name = name;
        this->address = address;
        this->nationKey = nationKey;
        this->phone = phone;
        this->accbal = accbal;
        this->comment = comment;
    }

    double getAccbal() const {
        return accbal;
    }

    void setAccbal(double accbal) {
        this->accbal = accbal;
    }

    const pdb::String& getAddress() const {
        return address;
    }

    void setAddress(const pdb::String& address) {
        this->address = address;
    }

    const pdb::String& getComment() const {
        return comment;
    }

    void setComment(const pdb::String& comment) {
        this->comment = comment;
    }

    const pdb::String& getName() const {
        return name;
    }

    void setName(const pdb::String& name) {
        this->name = name;
    }

    int getNationKey() const {
        return nationKey;
    }

    void setNationKey(int nationKey) {
        this->nationKey = nationKey;
    }

    const pdb::String& getPhone() const {
        return phone;
    }

    void setPhone(const pdb::String& phone) {
        this->phone = phone;
    }

    int getSupplierKey() const {
        return supplierKey;
    }

    void setSupplierKey(int supplierKey) {
        this->supplierKey = supplierKey;
    }
};
#endif
