#ifndef AGGREGATION_PROCESSOR_H
#define AGGREGATION_PROCESSOR_H


#include "UseTemporaryAllocationBlock.h"
#include "InterfaceFunctions.h"
#include "AggregationMap.h"
#include "PDBMap.h"
#include "PDBVector.h"
#include "Handle.h"
#include "SimpleSingleTableQueryProcessor.h"

namespace pdb {

template <class KeyType, class ValueType>
class AggregationProcessor : public SimpleSingleTableQueryProcessor {

public:
    ~AggregationProcessor(){};
    AggregationProcessor(){};
    AggregationProcessor(HashPartitionID id);
    void initialize() override;
    void loadInputPage(void* pageToProcess) override;
    void loadInputObject(Handle<Object>& objectToProcess) override;
    void loadOutputPage(void* pageToWriteTo, size_t numBytesInPage) override;
    bool fillNextOutputPage() override;
    void finalize() override;
    void clearOutputPage() override;
    void clearInputPage() override;
    bool needsProcessInput() override;
    int getNumHashKeys() override;

private:
    UseTemporaryAllocationBlockPtr blockPtr;
    Handle<Vector<Handle<AggregationMap<KeyType, ValueType>>>> inputData;
    Handle<Map<KeyType, ValueType>> outputData;
    bool finalized;
    Handle<AggregationMap<KeyType, ValueType>> curMap;
    int id;

    // the iterators for current map partition
    PDBMapIterator<KeyType, ValueType>* begin;
    PDBMapIterator<KeyType, ValueType>* end;

    // statistics
    int numHashKeys;
};
}


#include "AggregationProcessor.cc"


#endif
