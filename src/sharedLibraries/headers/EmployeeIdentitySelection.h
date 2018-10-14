#ifndef EMPLOYEE_IDENTITY_SELECT_H
#define EMPLOYEE_IDENTITY_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "SharedEmployee.h"

// By Binhang, May 2017

using namespace pdb;
class EmployeeIdentitySelection : public SelectionComp<SharedEmployee, SharedEmployee> {

public:
    ENABLE_DEEP_COPY

    EmployeeIdentitySelection() {}

    Lambda<bool> getSelection(Handle<SharedEmployee> checkMe) override {
        return makeLambda(checkMe, [](Handle<SharedEmployee>& checkMe) { return true; });
    }

    Lambda<Handle<SharedEmployee>> getProjection(Handle<SharedEmployee> checkMe) override {
        return makeLambda(checkMe, [](Handle<SharedEmployee>& checkMe) {
            checkMe->print();

            Handle<SharedEmployee> newEmployee = makeObject<SharedEmployee>(
                *(checkMe->getName()), 100, checkMe->getSalary());  // cannot get age!
            newEmployee->print();
            return newEmployee;

            // return checkMe;
        });
    }
};


#endif