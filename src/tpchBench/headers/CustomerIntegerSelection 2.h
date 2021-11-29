#ifndef CUSTOMER_INTEGER_SELECT_H
#define CUSTOMER_INTEGER_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"

#include "Customer.h"

using namespace pdb;
class CustomerIntegerSelection : public SelectionComp<Customer, Customer> {

public:
    ENABLE_DEEP_COPY

    CustomerIntegerSelection() {}

    // Select the 9th Customer Object
    Lambda<bool> getSelection(Handle<Customer> checkMe) override {
        return makeLambdaFromMethod(checkMe, is9); 
    }

    // Return the Customer object
    Lambda<Handle<Customer>> getProjection(Handle<Customer> checkMe) override {

        return makeLambda(checkMe, [](Handle<Customer>& checkMe) {
            return checkMe;
        });
    }
};
#endif
