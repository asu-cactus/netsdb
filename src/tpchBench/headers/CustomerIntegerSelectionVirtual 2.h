#ifndef CUSTOMER_INTEGER_SELECT_VIRTUAL_H
#define CUSTOMER_INTEGER_SELECT_VIRTUAL_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"

#include "Customer.h"

using namespace pdb;
class CustomerIntegerSelectionVirtual : public SelectionComp<Customer, Customer> {

public:
    ENABLE_DEEP_COPY

    CustomerIntegerSelectionVirtual() {}

    // Select the 9th Customer Object
    Lambda<bool> getSelection(Handle<Customer> checkMe) override {
        return makeLambdaFromMethod(checkMe, is9Virtual); 
    }

    // Return the Customer object
    Lambda<Handle<Customer>> getProjection(Handle<Customer> checkMe) override {

        return makeLambda(checkMe, [](Handle<Customer>& checkMe) {
            return checkMe;
        });
    }
};
#endif
