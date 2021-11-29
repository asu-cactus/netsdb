#ifndef CUSTOMER_STRING_SELECT_VIRTUAL_H
#define CUSTOMER_STRING_SELECT_VIRTUAL_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"

#include "Customer.h"


using namespace pdb;
class CustomerStringSelectionVirtual : public SelectionComp<Customer, Customer> {

public:
    ENABLE_DEEP_COPY

    CustomerStringSelectionVirtual() {}

    // Select All Building Customer Objects
    Lambda<bool> getSelection(Handle<Customer> checkMe) override {
        return makeLambdaFromMethod(checkMe, isBuildingVirtual); 
    }

    // Return the Customer object
    Lambda<Handle<Customer>> getProjection(Handle<Customer> checkMe) override {

        return makeLambda(checkMe, [](Handle<Customer>& checkMe) {
            return checkMe;
        });
    }
};
#endif
