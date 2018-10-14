#ifndef CUSTOMER_STRING_SELECT_VIRTUAL_NOT_H
#define CUSTOMER_STRING_SELECT_VIRTUAL_NOT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"

#include "Customer.h"


using namespace pdb;
class CustomerStringSelectionVirtualNot : public SelectionComp<Customer, Customer> {

public:
    ENABLE_DEEP_COPY

    CustomerStringSelectionVirtualNot() {}

    // Select All non-Building Customer Objects
    Lambda<bool> getSelection(Handle<Customer> checkMe) override {
        return makeLambdaFromMethod(checkMe, notBuildingVirtual); 
    }

    // Return the Customer object
    Lambda<Handle<Customer>> getProjection(Handle<Customer> checkMe) override {

        return makeLambda(checkMe, [](Handle<Customer>& checkMe) {
            return checkMe;
        });
    }
};
#endif
