#ifndef SETOPERATIONSELECTION_H
#define SETOPERATIONSELECTION_H

#include "Handle.h"
#include "SetOperation.h"

namespace pdb {
class SetOperationSelection : public SetOperation {
    void execute(Handle<SetOperationAlgo> algo) override {
        algo->forSelection();
    }
};
}

#endif  // PDB_SETOPERATIONSELECTION_H
