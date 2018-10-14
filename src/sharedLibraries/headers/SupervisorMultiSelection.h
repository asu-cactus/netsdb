
#ifndef SUPERVISOR_MULTI_SELECT_H
#define SUPERVISOR_MULTI_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "MultiSelectionComp.h"
#include "Employee.h"
#include "Supervisor.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Supervisor.h"

using namespace pdb;
class SupervisorMultiSelection : public MultiSelectionComp<Employee, Supervisor> {

public:
    ENABLE_DEEP_COPY

    SupervisorMultiSelection() {}

    Lambda<bool> getSelection(Handle<Supervisor> checkMe) override {
        return makeLambdaFromMethod(checkMe, getSteve) == makeLambdaFromMember(checkMe, me);
    }

    Lambda<Vector<Handle<Employee>>> getProjection(Handle<Supervisor> checkMe) override {
        return makeLambdaFromMember(checkMe, myGuys);
    }
};


#endif
