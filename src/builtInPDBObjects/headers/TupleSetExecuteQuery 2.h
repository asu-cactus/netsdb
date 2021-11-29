
#ifndef TUPLE_SET_EXEC_QUERY_H
#define TUPLE_SET_EXEC_QUERY_H

#include "Object.h"

// PRELOAD %TupleSetExecuteQuery%

namespace pdb {

// encapsulates a request to run a query
class TupleSetExecuteQuery : public Object {

public:
    TupleSetExecuteQuery() {
        isAggregation = false;
    }
    ~TupleSetExecuteQuery() {}

    TupleSetExecuteQuery(bool isAggregation) {
        this->isAggregation = isAggregation;
    }
    bool getIsAggregation() {
        return isAggregation;
    }

    ENABLE_DEEP_COPY

private:
    bool isAggregation;
};
}

#endif
