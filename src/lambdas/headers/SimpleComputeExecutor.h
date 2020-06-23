
#ifndef SIMPLE_COMPUTE_EXEC_H
#define SIMPLE_COMPUTE_EXEC_H

#include "TupleSet.h"
#include "ComputeExecutor.h"
#include <memory>

namespace pdb {

class SimpleComputeExecutor;
typedef std::shared_ptr<SimpleComputeExecutor> SimpleComputeExecutorPtr;

// this is a simple generic implementation of a ComputeExecutor
class SimpleComputeExecutor : public ComputeExecutor {

private:
    // this is the output TupleSet that we return
    TupleSetPtr output;

    // this is a lambda that we'll call to process input
    std::function<TupleSetPtr(TupleSetPtr)> processInput;

    // JiaNote: this is for debugging purpose
    std::string myType;


public:
    SimpleComputeExecutor(TupleSetPtr outputIn,
                          std::function<TupleSetPtr(TupleSetPtr)> processInputIn,
                          std::string myTypeIn = "SimpleComputeExecutor") {
        output = outputIn;
        processInput = processInputIn;
        myType = myTypeIn;
    }


    TupleSetPtr process(TupleSetPtr input) override {
        if (input == nullptr) {
             return nullptr;
        }
        return processInput(input);
    }

    std::string getType() override {
        return myType;
    }
};
}

#endif
