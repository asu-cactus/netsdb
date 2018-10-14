
#ifndef ALL_SELECT_H
#define ALL_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "SharedEmployee.h"
#include "Supervisor.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "SharedEmployee.h"

using namespace pdb;
class AllSelection : public SelectionComp<String, SharedEmployee> {

public:
    ENABLE_DEEP_COPY

    AllSelection() {}

    Lambda<bool> getSelection(Handle<SharedEmployee> checkMe) override {
        return makeLambda(checkMe, [](Handle<SharedEmployee>& checkMe) { return true; });
    }

    Lambda<Handle<String>> getProjection(Handle<SharedEmployee> checkMe) override {
        return makeLambdaFromMethod(checkMe, getName);
    }
};


#endif
