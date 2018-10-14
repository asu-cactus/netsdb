
#ifndef ALL_SELECT_WITH_CREATION_H
#define ALL_SELECT_WITH_CREATION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "Employee.h"
#include "Supervisor.h"
#include "PDBVector.h"
#include "PDBString.h"

using namespace pdb;
class AllSelectionWithCreation : public SelectionComp<Employee, Employee> {

public:
    ENABLE_DEEP_COPY

    AllSelectionWithCreation() {}

    Lambda<bool> getSelection(Handle<Employee> checkMe) override {
        return makeLambda(checkMe, [](Handle<Employee>& checkMe) { return true; });
    }

    Lambda<Handle<Employee>> getProjection(Handle<Employee> checkMe) override {
        return makeLambda(checkMe, [](Handle<Employee>& checkMe) {
            // checkMe->print();
            Handle<Employee> newEmployee =
                makeObject<Employee>(*(checkMe->getName()),
                                     100,
                                     checkMe->department,
                                     checkMe->salary);  // cannot get age!
            // newEmployee->print();
            return newEmployee;
        });
    }
};


#endif
