
#ifndef JOIN_MAP_CC
#define JOIN_MAP_CC

#include "InterfaceFunctions.h"
#include "JoinMap.h"

namespace pdb {

template <class ValueType>
JoinMap<ValueType>::JoinMap(uint32_t initSize, size_t partitionId, int numPartitions) {

    if (initSize < 2) {
        initSize = 2;
    }

    // this way, we'll allocate extra bytes on the end of the array
    JoinMapRecordClass<ValueType> temp;
    size_t size = temp.getObjSize();
    myArray = makeObjectWithExtraStorage<JoinPairArray<ValueType>>(size * initSize, initSize);
    this->partitionId = partitionId;
    this->numPartitions = numPartitions;
}


template <class ValueType>
JoinMap<ValueType>::JoinMap(uint32_t initSize) {

    if (initSize < 2) {
        initSize = 2;
    }

    // this way, we'll allocate extra bytes on the end of the array
    JoinMapRecordClass<ValueType> temp;
    size_t size = temp.getObjSize();
    myArray = makeObjectWithExtraStorage<JoinPairArray<ValueType>>(size * initSize, initSize);
}

template <class ValueType>
JoinMap<ValueType>::JoinMap() {

    JoinMapRecordClass<ValueType> temp;
    size_t size = temp.getObjSize();
    myArray = makeObjectWithExtraStorage<JoinPairArray<ValueType>>(size * 2, 2);
}

template <class ValueType>
JoinMap<ValueType>::~JoinMap() {}

template <class ValueType>
void JoinMap<ValueType>::setUnused(const size_t& clearMe) {
    myArray->setUnused(clearMe);
}

template <class ValueType>
ValueType& JoinMap<ValueType>::push(const size_t& me) {
    if (myArray->isOverFull()) {
        Handle<JoinPairArray<ValueType>> temp = myArray->doubleArray();
        //std::cout << "doubled the size of JoinPairArray" << std::endl;
        myArray = temp;
    }
    return myArray->push(me);
}

template <class ValueType>
JoinRecordList<ValueType> JoinMap<ValueType>::lookup(const size_t& me) {
    return myArray->lookup(me);
}

template <class ValueType>
int JoinMap<ValueType>::count(const size_t& which) {
    return myArray->count(which);
}

template <class ValueType>
size_t JoinMap<ValueType>::size() const {
    return myArray->numUsedSlots();
}

template <class ValueType>
JoinMapIterator<ValueType> JoinMap<ValueType>::begin() {
    JoinMapIterator<ValueType> returnVal(myArray, true);
    return returnVal;
}

template <class ValueType>
JoinMapIterator<ValueType> JoinMap<ValueType>::end() {
    return JoinMapIterator<ValueType> ();
}

template <class ValueType>
size_t JoinMap<ValueType>::getPartitionId() {
    return this->partitionId;
}

template <class ValueType>
void JoinMap<ValueType>::setPartitionId(size_t partitionId) {
    this->partitionId = partitionId;
}

template <class ValueType>
int JoinMap<ValueType>::getNumPartitions() {
    return this->numPartitions;
}

template <class ValueType>
void JoinMap<ValueType>::setNumPartitions(int numPartitions) {
    this->numPartitions = numPartitions;
}


template <class ValueType>
size_t JoinMap<ValueType>::getObjectSize() {
    return this->objectSize;
}

template <class ValueType>
void JoinMap<ValueType>::setObjectSize() {
    JoinMapRecordClass<ValueType> temp;
    size_t objSize = temp.getObjSize();
    this->objectSize = objSize;
}


template <class ValueType>
void JoinMap<ValueType>::print() {
    JoinMapIterator<ValueType> iter = this->begin();
    while (iter != this->end()) {
        std::shared_ptr<JoinRecordList<ValueType>> l = *iter;
        std::cout << l->getHash() << ":";
        for (int i = 0; i < l->size(); i++) {
           std::cout << (*l)[i].myData.toString();
        }
        std::cout << std::endl;
        ++iter;
    }
}
}

#endif
