#ifndef AGGREGATION_MAP_CC
#define AGGREGATION_MAP_CC

#include "AggregationMap.h"

namespace pdb {

//wraps the container for shuffle data for Aggregation

template <class KeyType, class ValueType>
AggregationMap<KeyType, ValueType>::AggregationMap(uint32_t initSize) : Map<KeyType, ValueType>(initSize) {
/*
    if (initSize < 2) {
        std::cout << "Fatal Error: Map initialization:" << initSize
                  << " too small; must be at least one.\n";

        initSize = 2;
    }

    // this way, we'll allocate extra bytes on the end of the array
    MapRecordClass<KeyType, ValueType> temp;
    size_t size = temp.getObjSize();
    this->myArray =
        makeObjectWithExtraStorage<PairArray<KeyType, ValueType>>(size * initSize, initSize);
*/
}

template <class KeyType, class ValueType>
AggregationMap<KeyType, ValueType>::AggregationMap() : Map<KeyType, ValueType>(){
    //std::cout << "initialize AggregationMap()" << std::endl;
    //MapRecordClass<KeyType, ValueType> temp;
    //size_t size = temp.getObjSize();
    //std::cout << "size=" << size << std::endl;
    //std::cout << "make array:this->myArray = makeObjectWithExtraStorage<PairArray<KeyType, ValueType>>(size * 2, 2);" << std::endl;
    //this->myArray = makeObjectWithExtraStorage<PairArray<KeyType, ValueType>>(size * 2, 2);
}

template <class KeyType, class ValueType>
AggregationMap<KeyType, ValueType>::~AggregationMap() {}


template <class KeyType, class ValueType>
unsigned int AggregationMap<KeyType, ValueType>::getHashPartitionId() {
    return this->hashPartitionId;
}

template <class KeyType, class ValueType>
void AggregationMap<KeyType, ValueType>::setHashPartitionId(unsigned int partitionId) {
    this->hashPartitionId = partitionId;
}
}

#endif
