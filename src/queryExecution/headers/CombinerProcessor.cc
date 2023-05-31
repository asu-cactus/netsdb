#ifndef COMBINER_PROCESSOR_CC
#define COMBINER_PROCESSOR_CC

#include "CombinerProcessor.h"
#include "Employee.h"

namespace pdb {


template <class KeyType, class ValueType>
CombinerProcessor<KeyType, ValueType>::CombinerProcessor(std::vector<HashPartitionID>& partitions) {
    PDB_COUT << "running CombinerProcessor constructor" << std::endl;
    this->numNodePartitions = partitions.size();
    finalized = false;

    int i;
    for (i = 0; i < partitions.size(); i++) {
        nodePartitionIds.push_back(partitions[i]);
    }
    count = 0;
    curPartPos = 0;
    begin = nullptr;
    end = nullptr;
}

// initialize
template <class KeyType, class ValueType>
void CombinerProcessor<KeyType, ValueType>::initialize() {
    finalized = false;
}

// loads up another input page to process
template <class KeyType, class ValueType>
void CombinerProcessor<KeyType, ValueType>::loadInputPage(void* pageToProcess) {
    PDB_COUT << "CombinerProcessor: to load a new input page" << std::endl;
    Record<Vector<Handle<Map<KeyType, ValueType>>>>* myRec =
        (Record<Vector<Handle<Map<KeyType, ValueType>>>>*)pageToProcess;
    inputData = myRec->getRootObject();
    curPartPos = 0;
    count = 0;
    curPartId = nodePartitionIds[curPartPos];
    curMap = (*inputData)[curPartId];
    if (begin != nullptr) {
        delete begin;
        begin = nullptr;
    }
    if (end != nullptr) {
        delete end;
        end = nullptr;
    }
    std::cout << "CombinerProcessor: loaded a page with first partition id=" << curPartId
             << " and size=" << curMap->size() << std::endl;
    begin = new PDBMapIterator<KeyType, ValueType>(curMap->getArray(), true);
    if (begin == nullptr) {
        std::cout << "CombinerProcessor.cc: Failed to allocate memory" << std::endl;
        exit(1);
    }
    end = new PDBMapIterator<KeyType, ValueType>(curMap->getArray());
    if (end == nullptr) {
        std::cout << "CombinerProcessor.cc: Failed to allocate memory" << std::endl;
        exit(1);
    }
}

// loads up another output page to write results to
template <class KeyType, class ValueType>
void CombinerProcessor<KeyType, ValueType>::loadOutputPage(void* pageToWriteTo,
                                                           size_t numBytesInPage) {

    blockPtr = nullptr;
    blockPtr = std::make_shared<UseTemporaryAllocationBlock>(pageToWriteTo, numBytesInPage);
    std::cout << "numBytesInPage=" << numBytesInPage << std::endl;
    outputData =
        makeObject<Vector<Handle<AggregationMap<KeyType, ValueType>>>>(this->numNodePartitions);
    int i;
    for (i = 0; i < numNodePartitions; i++) {
        std::cout << "to create the " << i << "-th partition on this node" << std::endl;
        Handle<AggregationMap<KeyType, ValueType>> currentMap =
            makeObject<AggregationMap<KeyType, ValueType>>();
        HashPartitionID currentPartitionId = nodePartitionIds[i];
        std::cout << "currentPartitionId=" << currentPartitionId << std::endl;
        // however we only use the relative/local hash partition id
        currentMap->setHashPartitionId(i);
        outputData->push_back(currentMap);
    }
    curOutputMap = (*outputData)[curPartPos];
}

template <class KeyType, class ValueType>
bool CombinerProcessor<KeyType, ValueType>::fillNextOutputPage() {

    // if we are finalized, see if there are some left over records
    if (finalized) {
        for (int i = 0; i < numNodePartitions; i++) {
            std::cout << "outputData[" << i << "].size()=" << (*outputData)[i]->size() << std::endl;
            std::cout << "count=" << count << std::endl;
        }

        getRecord(outputData);
        return false;
    }

    // we are not finalized, so process the page
    try {

        // see if there are any more items in current map to iterate over
        while (true) {

            if (!((*begin) != (*end))) {
                if (curPartPos < numNodePartitions - 1) {
                    curPartPos++;
                    curPartId = nodePartitionIds[curPartPos];
                    curMap = (*inputData)[curPartId];
                    if (curMap->size() > 0) {
                        if (begin != nullptr) delete begin;
                        if (end != nullptr) delete end;
                        begin = new PDBMapIterator<KeyType, ValueType>(curMap->getArray(), true);
                        if (begin == nullptr) {
                            std::cout << "CombinerProcessor.cc: Failed to allocate memory" << std::endl;
                            exit(1);
                        }
                        end = new PDBMapIterator<KeyType, ValueType>(curMap->getArray());
                        if (end == nullptr) {
                            std::cout << "CombinerProcessor.cc: Failed to allocate memory" << std::endl;
                            exit(1);
                        }

                        if ((*begin) != (*end)) {
                            curOutputMap = (*outputData)[curPartPos];
                        } else {
                            std::cout << "this is strange: map size > 0 but begin == end"
                                     << std::endl;
                            continue;
                        }
                    } else {
                        continue;
                    }
                } else {
                    // JiaNote: this is important, we need make sure when we load next page, we
                    // start output from the 0-th partition
                    curPartPos = 0;
                    curOutputMap = (*outputData)[0];
                    return false;
                }
            }
            KeyType curKey = (*(*begin)).key;
            ValueType curValue = (*(*begin)).value;
            if (curOutputMap->count(curKey) == 0) {
            // if the key is not there

                ValueType* temp = nullptr;
                try {
                    temp = &((*curOutputMap)[curKey]);

                } catch (NotEnoughSpace& n) {

                    throw n;
                }
                try {

                    *temp = curValue;
                    ++(*begin);
                    count++;
		    std::cout << "inserted one new TreeResult object: " << count << std::endl;
                    // if we couldn't fit the value
                } catch (NotEnoughSpace& n) {
                    curOutputMap->setUnused(curKey);

                    throw n;
                }
                // the key is there
            } else {

                // get the value and copy of it
                ValueType& temp = (*curOutputMap)[curKey];
                ValueType copy = temp;

                // and add to old value, producing a new one
                try {
                    std::cout << "combining two TreeResult objects: " << count << std::endl;
                    temp = copy + curValue;
                    ++(*begin);
                    count++;
                    std::cout << "combined two TreeResult objects: " << count << std::endl;
                    // if we got here, it means we run out of RAM and we need to restore the old
                    // value in the destination hash map
                } catch (NotEnoughSpace& n) {
                    temp = copy;
                    throw n;
                }
            }
        }

    } catch (NotEnoughSpace& n) {
        for (int i = 0; i < numNodePartitions; i++) {
            std::cout << "outputData[" << i << "].size()=" << (*outputData)[i]->size() << std::endl;
        }
        getRecord(outputData);
        return true;
    }
}

template <class KeyType, class ValueType>
void CombinerProcessor<KeyType, ValueType>::finalize() {
   std::cout << "finalized a page" << std::endl;
   finalized = true;
}

template <class KeyType, class ValueType>
void CombinerProcessor<KeyType, ValueType>::clearOutputPage() {
    blockPtr = nullptr;
    outputData = nullptr;
    curOutputMap = nullptr;
}

template <class KeyType, class ValueType>
void CombinerProcessor<KeyType, ValueType>::clearInputPage() {
    inputData = nullptr;
}
}


#endif
