#ifndef CUSTOMER_STRING_SELECT_H
#define CUSTOMER_STRING_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"

#include "Customer.h"

using namespace pdb;
class CustomerStringSelection : public SelectionComp<Customer, Customer> {

public:
    ENABLE_DEEP_COPY

    CustomerStringSelection() {}

    // Select All Building Customer Objects
    Lambda<bool> getSelection(Handle<Customer> checkMe) override {
        return makeLambdaFromMethod(checkMe, isBuilding); 
    }

    // Return the Customer object
    Lambda<Handle<Customer>> getProjection(Handle<Customer> checkMe) override {

        return makeLambda(checkMe, [](Handle<Customer>& checkMe) {
            return checkMe;
        });
    }
};
#endif
