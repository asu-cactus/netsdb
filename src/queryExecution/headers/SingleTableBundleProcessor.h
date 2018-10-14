
#ifndef SINGLE_TABLE_BUNDLE_PROCESSOR_H
#define SINGLE_TABLE_BUNDLE_PROCESSOR_H

// by Jia, Oct 2016


#include "UseTemporaryAllocationBlock.h"
#include "InterfaceFunctions.h"
#include "PDBVector.h"
#include "Selection.h"
#include "Handle.h"
#include "GenericBlock.h"
#include "PipelineContext.h"
#include <memory>


namespace pdb {
class SingleTableBundleProcessor;
typedef shared_ptr<SingleTableBundleProcessor> SingleTableBundleProcessorPtr;


class SingleTableBundleProcessor {

private:
    // this is the list of input objects
    Handle<Vector<Handle<Object>>> inputVec;

    // this is where we are in the input
    size_t posInInput;

    // this is where the output objects are put
    Handle<GenericBlock> outputBlock;

    // tells whether we have been finalized
    bool finalized;

    // size of objects in the output GenericBlock
    size_t batchSize;

    // pipeline context
    PipelineContextPtr context;

    long totalSize;

public:
    ~SingleTableBundleProcessor();
    SingleTableBundleProcessor();
    void initialize();
    void loadInputPage(void* pageToProcess);
    Handle<GenericBlock> loadOutputBlock(size_t batchSize);
    bool fillNextOutputBlock();
    void finalize();
    void clearOutputBlock();
    void clearInputPage();
    void setContext(PipelineContextPtr context);
    PipelineContextPtr getContext();
};
}

#endif
