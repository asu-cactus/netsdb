
#ifndef QUERY_EXEC_H
#define QUERY_EXEC_H

#include "TupleSet.h"

namespace pdb {

// a nice little typedef to shared_ptrs to QueryExecutor objects
class QueryExecutor;
typedef std::shared_ptr<QueryExecutor> QueryExecutorPtr;

// this is a QueryExecutor.  By definition, it has one method that takes an input a TupleSet, and
// then somehow transofrms it to create a new TupleSet (a TupleSet is a column-oriented list of
// tuples)
class QueryExecutor {

public:
    virtual TupleSetPtr process(TupleSetPtr input) = 0;
};
}

#endif
