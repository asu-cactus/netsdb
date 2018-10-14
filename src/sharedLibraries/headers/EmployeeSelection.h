
#ifndef EMPLOYEE_SELECT_H
#define EMPLOYEE_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "SharedEmployee.h"
#include "Supervisor.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "SharedEmployee.h"

using namespace pdb;
class EmployeeSelection : public SelectionComp<String, SharedEmployee> {

public:
    ENABLE_DEEP_COPY

    EmployeeSelection() {}

    Lambda<bool> getSelection(Handle<SharedEmployee> checkMe) override {
        return makeLambdaFromMethod(checkMe, isFrank);
    }

    Lambda<Handle<String>> getProjection(Handle<SharedEmployee> checkMe) override {
        return makeLambdaFromMethod(checkMe, getName);
    }
};


#endif
