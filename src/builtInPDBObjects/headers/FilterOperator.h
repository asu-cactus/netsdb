#ifndef FILTER_OPERATOR_H
#define FILTER_OPERATOR_H

// by Jia, Sept 2016

#include "ExecutionOperator.h"
#include "QueryBase.h"
#include "Selection.h"
#include "BlockQueryProcessor.h"

// PRELOAD %FilterOperator%

namespace pdb {

/*
 * this class encapsulates a FilterOperator that runs in backend.
 */

class FilterOperator : public ExecutionOperator {

public:
    ENABLE_DEEP_COPY

    FilterOperator() {
        selection = nullptr;
    }

    FilterOperator(Handle<QueryBase>& selection) {
        this->selection = selection;
    }

    std::string getName() override {
        return "FilterOperator";
    }

    BlockQueryProcessorPtr getProcessor() override {
        if (selection == nullptr) {
            return nullptr;
        }
        return (unsafeCast<Selection<Object, Object>>(selection))->getFilterBlockProcessor();
    }

    Handle<QueryBase>& getSelection() override {
        return this->selection;
    }

private:
    Handle<QueryBase> selection;
};
}

#endif
