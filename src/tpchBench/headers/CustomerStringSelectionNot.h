#ifndef CUSTOMER_STRING_SELECT_NOT_H
#define CUSTOMER_STRING_SELECT_NOT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"

#include "Customer.h"

using namespace pdb;
class CustomerStringSelectionNot : public SelectionComp<Customer, Customer> {

public:
    ENABLE_DEEP_COPY

    CustomerStringSelectionNot() {}

    // Select All non-building Customer Objects
    Lambda<bool> getSelection(Handle<Customer> checkMe) override {
        return makeLambdaFromMethod(checkMe, notBuilding); 
    }

    // Return the Customer object
    Lambda<Handle<Customer>> getProjection(Handle<Customer> checkMe) override {

        return makeLambda(checkMe, [](Handle<Customer>& checkMe) {
            return checkMe;
        });
    }
};
#endif
