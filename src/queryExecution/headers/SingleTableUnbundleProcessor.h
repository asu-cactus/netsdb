
#ifndef SINGLE_TABLE_UNBUNDLE_PROCESSOR_H
#define SINGLE_TABLE_UNBUNDLE_PROCESSOR_H

#include "UseTemporaryAllocationBlock.h"
#include "InterfaceFunctions.h"
#include "PDBVector.h"
#include "Selection.h"
#include "Handle.h"
#include "GenericBlock.h"
#include "PipelineContext.h"
#include <memory>

namespace pdb {

class SingleTableUnbundleProcessor;

typedef std::shared_ptr<SingleTableUnbundleProcessor> SingleTableUnbundleProcessorPtr;


class SingleTableUnbundleProcessor {

private:
    // this is the block of input objects
    Handle<GenericBlock> inputBlock;

    // this is where the output objects are put
    Handle<Vector<Handle<Object>>> outputVec;

    // this is where we are in the input
    size_t posInInput;

    // tells whether we have been finalized
    bool finalized;

    // pipeline context
    PipelineContextPtr context;

public:
    ~SingleTableUnbundleProcessor();
    SingleTableUnbundleProcessor();
    void initialize();
    void loadInputBlock(Handle<GenericBlock> inputBlock);
    void loadOutputVector();
    bool fillNextOutputVector();
    void finalize();
    void clearOutputVec();
    void clearInputBlock();
    void setContext(PipelineContextPtr context);
    PipelineContextPtr getContext();
};
}

#endif
