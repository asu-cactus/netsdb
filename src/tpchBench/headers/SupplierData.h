#ifndef CUSTOMER_SUPPLIER_PART_AGG_H
#define CUSTOMER_SUPPLIER_PART_AGG_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"

using namespace pdb;
// This class represents a triple that holds a triple of (customerName, SupplierName, PartID)

class SupplierData : public pdb::Object {

public:
    String supplierName;
    Map<String, Vector<int>> soldPartIDs;

    ENABLE_DEEP_COPY

    // Default constructor:
    SupplierData() {}

    // Default destructor:
    ~SupplierData() {}

    // Constructor with arguments:
    SupplierData(pdb::String supplierName) {
        this->supplierName = supplierName;
        // this->soldPartIDs = pdb::makeObject<pdb::Map<pdb::String, pdb::Vector<int>>>();
    }

    String& getKey() {
        return supplierName;
    }

    Map<String, Vector<int>>& getValue() {
        return soldPartIDs;
    }

    /*void addSupplierPart(pdb::String supplierName, int partKey) {

        if(soldPartIDs->count(supplierName)==0) {
            // not found
            pdb::Handle<pdb::Vector<int>> partKeyVector = pdb::makeObject<pdb::Vector<int>>();
            partKeyVector->push_back(partKey);
            (*soldPartIDs)[supplierName] = * partKeyVector;

        } else {
            // found
            pdb::Vector<int> existing_partKeyVector = (*soldPartIDs)[supplierName];
            existing_partKeyVector.push_back(partKey);
            (*soldPartIDs)[supplierName] = existing_partKeyVector;
        }
    }*/

    Map<String, Vector<int>> getSoldPartIDs() {
        return soldPartIDs;
    }

    void setSoldPartIDs(Map<String, Vector<int>> soldPartIDs) {
        this->soldPartIDs = soldPartIDs;
    }

    int print() {
        std::cout << "SupplierName: " << supplierName << " [ ";
        auto iter = soldPartIDs.begin();
        int count = 0;
        while (iter != soldPartIDs.end()) {
            pdb::String customerName = (*iter).key;
            pdb::Vector<int> partIDs = soldPartIDs[customerName];
            std::cout << "Customer: " << customerName.c_str() << " (";
            for (int i = 0; i < partIDs.size(); ++i) {
                std::cout << " " << partIDs[i] << ",";
            }
            std::cout << ") ";
            ++iter;
            count++;
        }
        std::cout << "  ] " << std::endl;
        std::cout << " the count is " << count << std::endl;
        return count;
    }

    /*const pdb::String& getCustomerName() const {
        return supplierName;
    }

    void setCustomerName(const pdb::String& customerName) {
        this->supplierName = customerName;
    }*/
};

#endif
