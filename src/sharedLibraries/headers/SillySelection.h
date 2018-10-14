
#ifndef SILLY_SELECT_H
#define SILLY_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "Employee.h"
#include "Supervisor.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Supervisor.h"

using namespace pdb;
class SillySelection : public SelectionComp<Employee, Supervisor> {

public:
    ENABLE_DEEP_COPY

    SillySelection() {}

    Lambda<bool> getSelection(Handle<Supervisor> checkMe) override {
        return makeLambdaFromMethod(checkMe, getSteve) == makeLambdaFromMember(checkMe, me);
    }

    Lambda<Handle<Employee>> getProjection(Handle<Supervisor> checkMe) override {
        return makeLambdaFromMethod(checkMe, getMe);
    }
};


#endif
