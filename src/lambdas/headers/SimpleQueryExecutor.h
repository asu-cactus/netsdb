
#ifndef SIMPLE_QUERY_EXEC_H
#define SIMPLE_QUERY_EXEC_H

#include "TupleSet.h"
#include "QueryExecutor.h"
#include <memory>

namespace pdb {

class SimpleQueryExecutor;
typedef std::shared_ptr<SimpleQueryExecutor> SimpleQueryExecutorPtr;

// this is a simple generic implementation of a QueryExecutor
class SimpleQueryExecutor : public QueryExecutor {

private:
    // this is the output TupleSet that we return
    TupleSetPtr output;

    // this is a lambda that we'll call to process input
    std::function<TupleSetPtr(TupleSetPtr)> processInput;

public:
    SimpleQueryExecutor(TupleSetPtr outputIn,
                        std::function<TupleSetPtr(TupleSetPtr)> processInputIn) {
        output = outputIn;
        processInput = processInputIn;
    }

    TupleSetPtr process(TupleSetPtr input) override {
        return processInput(input);
    }
};
}

#endif
