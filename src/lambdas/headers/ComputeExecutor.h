
#ifndef COMP_EXEC_H
#define COMP_EXEC_H

#include "TupleSet.h"

namespace pdb {

// a nice little typedef to shared_ptrs to CompExecutor objects
class ComputeExecutor;
typedef std::shared_ptr<ComputeExecutor> ComputeExecutorPtr;

// this is a ComputeExecutor.  By definition, it has one method that takes an input a TupleSet, and
// then somehow transofrms it to create a new TupleSet (a TupleSet is a column-oriented list of
// tuples).
class ComputeExecutor {

public:
    // precess a tuple set
    virtual TupleSetPtr process(TupleSetPtr input) = 0;

    // JiaNote: add below function for debugging
    virtual std::string getType() {
        return "UNKNOWN";
    };
};
}

#endif
