#ifndef AGGREGATION_PROCESSOR_CC
#define AGGREGATION_PROCESSOR_CC

#include "AggregationProcessor.h"

namespace pdb {


template <class KeyType, class ValueType>
AggregationProcessor<KeyType, ValueType>::~AggregationProcessor() {

    if(begin != nullptr)
        delete begin;
    if(end != nullptr)
        delete end;
}

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
        if (begin == nullptr) {
            std::cout << "AggregationProcessor.cc: Failed to allocate memory" << std::endl;
            exit(1);
        }
        end = new PDBMapIterator<KeyType, ValueType>(curMap->getArray());
        if (end == nullptr) {
            std::cout << "AggregationProcessor.cc: Failed to allocate memory" << std::endl;
            exit(1);
        }
    } else {
        // there is no hash partition for this thread
        curMap = nullptr;
    }
}


// loads up another input page to process
template <class KeyType, class ValueType>
void AggregationProcessor<KeyType, ValueType>::loadInputPage(void* pageToProcess) {
    Record<Vector<Handle<AggregationMap<KeyType, ValueType>>>>* myRec =
        (Record<Vector<Handle<AggregationMap<KeyType, ValueType>>>>*)pageToProcess;
    inputData = myRec->getRootObject();
    int numPartitions = inputData->size();
    int i;
    for (i = 0; i < numPartitions; i++) {
        curMap = (*inputData)[i];
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
    blockPtr = nullptr;
    blockPtr = std::make_shared<UseTemporaryAllocationBlock>(pageToWriteTo, numBytesInPage);
    outputData = makeObject<Map<KeyType, ValueType>>(2);
}

template <class KeyType, class ValueType>
bool AggregationProcessor<KeyType, ValueType>::fillNextOutputPage() {

    // if we are finalized, see if there are some left over records
    if (finalized) {
        getRecord(outputData);
        return false;
    }


    if (curMap == nullptr) {
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

		try {
                    temp = &((*outputData)[curKey]);
		} catch (NotEnoughSpace& n) {
		    throw n;
		}
                try {
                    *temp = curValue;
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
                    // if we got here, it means we run out of RAM and we need to restore the old
                    // value in the destination hash map
                } catch (NotEnoughSpace& n) {
                    temp = copy;
                    throw n;
                }
            }
            ++(*begin);
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
