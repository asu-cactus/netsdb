//
// Created by barnett on 10/2/16.
//

#ifndef PDB_PROCESSORFACTORYFILTERQUERYPROCESSOR_H
#define PDB_PROCESSORFACTORYFILTERQUERYPROCESSOR_H

#include "Selection.h"
#include "Handle.h"
#include "ProcessorFactory.h"
#include "QueryBase.h"

using std::make_shared;

namespace pdb {
class ProcessorFactoryProjectionQueryProcessor : public ProcessorFactory {
public:
    ProcessorFactoryProjectionQueryProcessor() {
        // for deep copy
    }

    ProcessorFactoryProjectionQueryProcessor(Handle<QueryBase> originalSelection)
        : _originalSelection(originalSelection) {}

    SimpleSingleTableQueryProcessorPtr makeProcessor() override {
        return make_shared<ProjectionQueryProcessor<Object, Object>>(_originalSelection);
    }

    ENABLE_DEEP_COPY

private:
    Handle<QueryBase> _originalSelection;
};
}

#endif  // PDB_PROCESSORFACTORYFILTERQUERYPROCESSOR_H
