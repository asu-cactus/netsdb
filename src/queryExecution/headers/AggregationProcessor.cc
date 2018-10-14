#ifndef AGGREGATION_PROCESSOR_CC
#define AGGREGATION_PROCESSOR_CC

#include "AggregationProcessor.h"

namespace pdb {


template <class KeyType, class ValueType>
AggregationProcessor<KeyType, ValueType>::AggregationProcessor(HashPartitionID id) {

    this->id = id;
    finalized = false;
    numHashKeys = 0;
    begin = nullptr;
    end = nullptr;
}

// initialize
template <class KeyType, class ValueType>
void AggregationProcessor<KeyType, ValueType>::initialize() {
    finalized = false;
}

template <class KeyType, class ValueType>
void AggregationProcessor<KeyType, ValueType>::loadInputObject(Handle<Object>& objectToProcess) {
    curMap = unsafeCast<AggregationMap<KeyType, ValueType>, Object>(objectToProcess);
    HashPartitionID hashIdForCurrentMap = curMap->getHashPartitionId();
    if (curMap->getHashPartitionId() == id) {
        if (begin != nullptr) {
            delete begin;
        }
        if (end != nullptr) {
            delete end;
        }
        begin = new PDBMapIterator<KeyType, ValueType>(curMap->getArray(), true);
        end = new PDBMapIterator<KeyType, ValueType>(curMap->getArray());

    } else {
        // there is no hash partition for this thread
        curMap = nullptr;
    }
}


// loads up another input page to process
template <class KeyType, class ValueType>
void AggregationProcessor<KeyType, ValueType>::loadInputPage(void* pageToProcess) {
    PDB_COUT << "AggregationProcessor-" << id << ": Loading input page" << std::endl;
    Record<Vector<Handle<AggregationMap<KeyType, ValueType>>>>* myRec =
        (Record<Vector<Handle<AggregationMap<KeyType, ValueType>>>>*)pageToProcess;
    inputData = myRec->getRootObject();
    int numPartitions = inputData->size();
    int i;
    for (i = 0; i < numPartitions; i++) {
        curMap = (*inputData)[i];
        HashPartitionID hashIdForCurrentMap = curMap->getHashPartitionId();
        PDB_COUT << i << "-th map's partitionId is " << hashIdForCurrentMap << std::endl;
        if (curMap->getHashPartitionId() == id) {
            PDB_COUT << "this map has my id = " << id << std::endl;
            if (begin != nullptr) {
                PDB_COUT << "we delete the begin iterator of last input page" << std::endl;
                delete begin;
            }
            if (end != nullptr) {
                PDB_COUT << "we delete the end iterator of last input page" << std::endl;
                delete end;
            }
            begin = new PDBMapIterator<KeyType, ValueType>(curMap->getArray(), true);
            end = new PDBMapIterator<KeyType, ValueType>(curMap->getArray());

            break;
        } else {
            // there is no hash partition for this thread
            curMap = nullptr;
        }
    }
}

// loads up another output page to write results to
template <class KeyType, class ValueType>
void AggregationProcessor<KeyType, ValueType>::loadOutputPage(void* pageToWriteTo,
                                                              size_t numBytesInPage) {
    PDB_COUT << "AggregationProcessor-" << id << ": Loading output page" << std::endl;
    blockPtr = nullptr;
    blockPtr = std::make_shared<UseTemporaryAllocationBlock>(pageToWriteTo, numBytesInPage);
    outputData = makeObject<Map<KeyType, ValueType>>();
}

template <class KeyType, class ValueType>
bool AggregationProcessor<KeyType, ValueType>::fillNextOutputPage() {

    // if we are finalized, see if there are some left over records
    if (finalized) {
        getRecord(outputData);
        return false;
    }


    if (curMap == nullptr) {
        PDB_COUT << "this page doesn't have my map with id = " << id << std::endl;
        return false;
    }


    // we are not finalized, so process the page
    try {

        // see if there are any more items in current map to iterate over
        while (true) {

            if (!((*begin) != (*end))) {
                return false;
            }
            KeyType curKey = (*(*begin)).key;
            ValueType curValue = (*(*begin)).value;
            // if the key is not there
            if (outputData->count(curKey) == 0) {
                ValueType* temp = nullptr;
                temp = &((*outputData)[curKey]);
                try {

                    *temp = curValue;
                    ++(*begin);
                    numHashKeys++;
                    // if we couldn't fit the value
                } catch (NotEnoughSpace& n) {
                    outputData->setUnused(curKey);
                    throw n;
                }
                // the key is there
            } else {
                // get the value and copy of it
                ValueType& temp = (*outputData)[curKey];
                ValueType copy = temp;

                // and add to old value, producing a new one
                try {

                    temp = copy + curValue;
                    ++(*begin);

                    // if we got here, it means we run out of RAM and we need to restore the old
                    // value in the destination hash map
                } catch (NotEnoughSpace& n) {
                    temp = copy;
                    throw n;
                }
            }
        }

    } catch (NotEnoughSpace& n) {
        getRecord(outputData);
        return true;
    }
}

template <class KeyType, class ValueType>
void AggregationProcessor<KeyType, ValueType>::finalize() {
    finalized = true;
}

template <class KeyType, class ValueType>
void AggregationProcessor<KeyType, ValueType>::clearOutputPage() {
    blockPtr = nullptr;
    outputData = nullptr;
}

template <class KeyType, class ValueType>
void AggregationProcessor<KeyType, ValueType>::clearInputPage() {
    curMap = nullptr;
    inputData = nullptr;
}

template <class KeyType, class ValueType>
bool AggregationProcessor<KeyType, ValueType>::needsProcessInput() {
    if (curMap == nullptr) {
        return false;
    } else {
        return true;
    }
}

template <class KeyType, class ValueType>
int AggregationProcessor<KeyType, ValueType>::getNumHashKeys() {
    return numHashKeys;
}

}


#endif
