#ifndef CUSTOMER_INTEGER_SELECT_NOT_H
#define CUSTOMER_INTEGER_SELECT_NOT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"

#include "Customer.h"

using namespace pdb;
class CustomerIntegerSelectionNot : public SelectionComp<Customer, Customer> {

public:
    ENABLE_DEEP_COPY

    CustomerIntegerSelectionNot() {}

    // Select all Customer Objects except the 9-th one
    Lambda<bool> getSelection(Handle<Customer> checkMe) override {
        return makeLambdaFromMethod(checkMe, not9); 
    }

    // Return the Customer object
    Lambda<Handle<Customer>> getProjection(Handle<Customer> checkMe) override {

        return makeLambda(checkMe, [](Handle<Customer>& checkMe) {
            return checkMe;
        });
    }
};
#endif
