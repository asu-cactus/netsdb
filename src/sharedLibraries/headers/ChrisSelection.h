
#ifndef CHRIS_SELECT_H
#define CHRIS_SELECT_H

#include "Selection.h"
#include "Employee.h"
#include "Supervisor.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "SharedEmployee.h"

using namespace pdb;
class ChrisSelection : public Selection<String, SharedEmployee> {

public:
    ENABLE_DEEP_COPY

    ChrisSelection() {}

    SimpleLambda<bool> getSelection(Handle<SharedEmployee>& checkMe) override {
        return makeSimpleLambda(checkMe, [&]() {
            // std :: cout << std :: to_string((*(checkMe->getName ()) != "Joe Johnson48")) << "; ";
            return (checkMe->isFrank());
        });
    }


    SimpleLambda<bool> getProjectionSelection(Handle<String>& checkMe) override {
        return makeSimpleLambda(checkMe, [&]() {
            // std :: cout << std :: to_string((*(checkMe->getName ()) != "Joe Johnson48")) << "; ";
            return ((*checkMe) == "Frank");
        });
    }


    SimpleLambda<Handle<String>> getProjection(Handle<SharedEmployee>& checkMe) override {
        return makeSimpleLambda(checkMe, [&] {
            // std :: cout << *(checkMe->getName ()) << std :: endl;
            return checkMe->getName();
        });
    }
};


#endif
