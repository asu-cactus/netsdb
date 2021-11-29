
#ifndef EXEC_QUERY_H
#define EXEC_QUERY_H

#include "Object.h"

// PRELOAD %ExecuteQuery%

namespace pdb {

// encapsulates a request to run a query
class ExecuteQuery : public Object {

public:
    ExecuteQuery() {}
    ~ExecuteQuery() {}

    ENABLE_DEEP_COPY
};
}

#endif
