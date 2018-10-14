
#ifndef PROJECTION_BLOCK_QUERY_PROCESSOR_CC
#define PROJECTION_BLOCK_QUERY_PROCESSOR_CC

#include "PDBDebug.h"
#include "InterfaceFunctions.h"
#include "ProjectionBlockQueryProcessor.h"


namespace pdb {

template <class Output, class Input>
ProjectionBlockQueryProcessor<Output, Input>::~ProjectionBlockQueryProcessor() {
    // std :: cout << "running ProjectionBlockQueryProcessor destructor" << std :: endl;
    this->inputBlock = nullptr;
    this->outputBlock = nullptr;
    this->context = nullptr;
    this->inputObject = nullptr;
}

template <class Output, class Input>
ProjectionBlockQueryProcessor<Output, Input>::ProjectionBlockQueryProcessor(
    Selection<Output, Input>& forMe) {

    // get a copy of the lambdas for query processing
    projection = forMe.getProjection(inputObject);
    finalized = false;
}


template <class Output, class Input>
ProjectionBlockQueryProcessor<Output, Input>::ProjectionBlockQueryProcessor(
    SimpleLambda<Handle<Output>> projection) {

    // get a copy of the lambdas for query processing
    this->projection = projection;
    finalized = false;
}

// no need to do anything
template <class Output, class Input>
void ProjectionBlockQueryProcessor<Output, Input>::initialize() {
    projectionFunc = projection.getFunc();
    finalized = false;
}

// loads up another input page to process
template <class Output, class Input>
void ProjectionBlockQueryProcessor<Output, Input>::loadInputBlock(Handle<GenericBlock> inputBlock) {
    this->inputBlock = inputBlock;
    this->batchSize = this->inputBlock->getBlock().size();
    posInInput = 0;
}

// load up another output page to process
template <class Output, class Input>
Handle<GenericBlock>& ProjectionBlockQueryProcessor<Output, Input>::loadOutputBlock() {
    this->outputBlock = makeObject<GenericBlock>();
    return this->outputBlock;
}

template <class Output, class Input>
bool ProjectionBlockQueryProcessor<Output, Input>::fillNextOutputBlock() {

    // std :: cout << "Projection processor is running" << std :: endl;
    Vector<Handle<Input>>& myInVec = (inputBlock->getBlock());
    Vector<Handle<Output>>& myOutVec = (outputBlock->getBlock());

    // if we are finalized, see if there are some left over records
    if (finalized) {
        return false;
    }

    // int totalObjects = 0;
    // we are not finalized, so process the page
    try {
        int vecSize = myInVec.size();
        // std :: cout << "Projection processor: posInInput="<< posInInput <<", input object num="
        // << vecSize << std :: endl;
        for (; posInInput < vecSize; posInInput++) {
            inputObject = myInVec[posInInput];
            myOutVec.push_back(projectionFunc());
            // totalObjects ++;
        }
        // std :: cout << "Projection processor processed one input block with "<<totalObjects << "
        // objects" << std :: endl;
        return false;

    } catch (NotEnoughSpace& n) {
        // std :: cout << "Projection processor consumed the page with "<< totalObjects << "
        // objects" << std :: endl;
        // std :: cout << "posInInput = " << posInInput << std :: endl;
        if (this->context != nullptr) {
            // getRecord (this->context->outputVec);
            context->setOutputFull(true);
        }
        return true;
    }
}

// must be called repeately after all of the input pages have been sent in...
template <class Output, class Input>
void ProjectionBlockQueryProcessor<Output, Input>::finalize() {
    finalized = true;
}

// must be called before freeing the memory in output page
template <class Output, class Input>
void ProjectionBlockQueryProcessor<Output, Input>::clearOutputBlock() {
    outputBlock = nullptr;
}

template <class Output, class Input>
void ProjectionBlockQueryProcessor<Output, Input>::clearInputBlock() {
    inputBlock = nullptr;
    inputObject = nullptr;
}
}

#endif
