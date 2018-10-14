
#ifndef GENERIC_WORK_H
#define GENERIC_WORK_H

#include "PDBWork.h"

// This template is used to make a simple piece of work with a particular execute function, and some
// state
namespace pdb {

class GenericWork : public PDBWork {

public:
    // this accepts the lambda that is used to process the RequestType object
    GenericWork(function<void(PDBBuzzerPtr)> executeMeIn) {
        executeMe = executeMeIn;
    }

    void execute(PDBBuzzerPtr callerBuzzer) {
        executeMe(callerBuzzer);
    }

private:
    function<void(PDBBuzzerPtr)> executeMe = nullptr;
};
}

#endif
