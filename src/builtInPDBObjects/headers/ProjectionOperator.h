#ifndef PROJECTION_OPERATOR_H
#define PROJECTION_OPERATOR_H

// by Jia, Sept 2016

#include "QueryBase.h"
#include "Selection.h"
#include "BlockQueryProcessor.h"
#include "ExecutionOperator.h"

// PRELOAD %ProjectionOperator%

namespace pdb {

/*
 * this class encapsulates a ProjectionOperator that runs in backend.
 */

class ProjectionOperator : public ExecutionOperator {

public:
    ENABLE_DEEP_COPY
    ProjectionOperator() {
        selection = nullptr;
    }


    ProjectionOperator(Handle<QueryBase>& selection) {
        this->selection = selection;
    }

    std::string getName() override {
        return "ProjectionOperator";
    }

    BlockQueryProcessorPtr getProcessor() override {
        if (selection == nullptr) {
            return nullptr;
        }
        return (unsafeCast<Selection<Object, Object>>(selection))->getProjectionBlockProcessor();
    }

    Handle<QueryBase>& getSelection() override {
        return this->selection;
    }


private:
    Handle<QueryBase> selection;
};
}

#endif
