#ifndef VIRTUAL_CUSTOMER_H
#define VIRTUAL_CUSTOMER_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"

#include "Customer.h"

// This class represents a Customer Object according to the TPC-H Database Benchmark

//	CREATE TABLE CUSTOMER (
//		C_CUSTKEY		SERIAL PRIMARY KEY,
//		C_NAME			VARCHAR(25),
//		C_ADDRESS		VARCHAR(40),
//		C_NATIONKEY		BIGINT NOT NULL, -- references N_NATIONKEY
//		C_PHONE			CHAR(15),
//		C_ACCTBAL		DECIMAL,
//		C_MKTSEGMENT	CHAR(10),
//		C_COMMENT		VARCHAR(117)
//	);


class VirtualCustomer : public Customer {


public:

    ENABLE_DEEP_COPY

    ~VirtualCustomer() {}

    VirtualCustomer() {}

    // Constructor with arguments using std::string

    VirtualCustomer(pdb::Vector<Order> orders,
             int custKey,
             std::string name,
             std::string address,
             int nationKey,
             std::string phone,
             double accbal,
             std::string mktsegment,
             std::string comment) {
        this->orders = orders;
        this->custKey = custKey;
        this->name = name;
        this->address = address;
        this->nationKey = nationKey;
        this->phone = phone;
        this->accbal = accbal;
        this->mktsegment = mktsegment;
        this->comment = comment;
    }

    __attribute__((noinline))
    bool is9Virtual() override {
        return custKey == 9;
    }


    __attribute__((noinline))
    bool not9Virtual() override {
        return custKey != 9;
    }

    __attribute__((noinline))
    bool isBuildingVirtual() override {
        return mktsegment == "BUILDING";
    }

    __attribute__((noinline))
    bool notBuildingVirtual() override {
        return mktsegment != "BUILDING";
    }


};
#endif
