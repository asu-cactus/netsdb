#ifndef EXECUTION_OPERATOR_H
#define EXECUTION_OPERATOR_H

// by Jia, Sept 2016
#include "BlockQueryProcessor.h"
#include "Object.h"
#include "QueryBase.h"
#include <string>

// PRELOAD %ExecutionOperator%

namespace pdb {

/*
 * this class encapsulates an Operator that can run in backend, now we support following operators:
 * Selection/Filtering
 * Projection
 */

class ExecutionOperator : public Object {

public:
    virtual string getName() = 0;
    virtual BlockQueryProcessorPtr getProcessor() = 0;
    virtual Handle<QueryBase>& getSelection() = 0;
};
}

#endif
