#ifndef AGGOUT_PROCESSOR_H
#define AGGOUT_PROCESSOR_H


#include "UseTemporaryAllocationBlock.h"
#include "InterfaceFunctions.h"
#include "PDBMap.h"
#include "PDBVector.h"
#include "Handle.h"
#include "SimpleSingleTableQueryProcessor.h"

namespace pdb {

template <class OutputClass, class KeyType, class ValueType>
class AggOutProcessor : public SimpleSingleTableQueryProcessor {

public:
    ~AggOutProcessor(){};
    AggOutProcessor();
    void initialize() override;
    void loadInputPage(void* pageToProcess) override;
    void loadOutputPage(void* pageToWriteTo, size_t numBytesInPage) override;
    bool fillNextOutputPage() override;
    void finalize() override;
    void clearOutputPage() override;
    void clearInputPage() override;

protected:
    UseTemporaryAllocationBlockPtr blockPtr;
    Handle<Map<KeyType, ValueType>> inputData;
    Handle<Vector<Handle<OutputClass>>> outputData;
    bool finalized;

    // the iterators for current map partition
    PDBMapIterator<KeyType, ValueType>* begin;
    PDBMapIterator<KeyType, ValueType>* end;

    // current pos in output vector
    int pos;
};
}


#include "AggOutProcessor.cc"


#endif
