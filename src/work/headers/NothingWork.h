/*
 * File:   NothingWork.h
 * Author: Chris
 *
 * Created on September 25, 2015, 5:15 PM
 */

#ifndef NOTHINGWORK_H
#define NOTHINGWORK_H

#include "PDBBuzzer.h"
#include "PDBCommWork.h"

// do no work

namespace pdb {

class NothingWork : public PDBWork {
public:
    NothingWork() {}

    void execute(PDBBuzzerPtr callerBuzzer) override {
        callerBuzzer = nullptr;
        /* so we don't get a compiler error */
    }
};
}

#endif /* NOTHINGWORK_H */
