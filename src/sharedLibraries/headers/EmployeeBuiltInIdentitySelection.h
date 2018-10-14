#ifndef EMPLOYEE_BUILTIN_IDENTITY_SELECT_H
#define EMPLOYEE_BUILTIN_IDENTITY_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Employee.h"
#include "SharedEmployee.h"

// By Binhang, May 2017

using namespace pdb;
class EmployeeBuiltInIdentitySelection : public SelectionComp<Employee, SharedEmployee> {

public:
    ENABLE_DEEP_COPY

    EmployeeBuiltInIdentitySelection() {}

    Lambda<bool> getSelection(Handle<SharedEmployee> checkMe) override {
        return makeLambda(checkMe, [](Handle<SharedEmployee>& checkMe) { return true; });
    }

    Lambda<Handle<Employee>> getProjection(Handle<SharedEmployee> checkMe) override {
        return makeLambda(checkMe, [](Handle<SharedEmployee>& checkMe) {
            checkMe->print();

            Handle<Employee> newEmployee =
                makeObject<Employee>(*(checkMe->getName()), 100);  // cannot get age!
            newEmployee->print();
            return newEmployee;

            // return checkMe;
        });
    }
};


#endif