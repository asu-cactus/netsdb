
#ifndef PARTITIONED_VECTOR_TUPLESET_ITER_H
#define PARTITIONED_VECTOR_TUPLESET_ITER_H

//#include "LambdaHelperClasses.h"

namespace pdb {

// this class iterates over an input pdb :: Vector, breaking it up into a series of TupleSet objects
template <class ValueType>
class PartitionedVectorTupleSetIterator : public ComputeSource {

private:
    int myPartitionId;

    int numPartitionsPerNode;

    int numNodes;

    GenericLambdaObjectPtr partitionLambda;

    // function to call to get another vector to process
    std::function<PDBPagePtr()> getAnotherVector;

    // function to call to free the vector
    std::function<void(PDBPagePtr)> doneWithVector;

    // this is the vector to process
    Handle<Vector<Handle<ValueType>>> iterateOverMe;

    // the pointer to the current page holding the vector, and the last page that we previously
    // processed
    Record<Vector<Handle<ValueType>>>*myRec, *lastRec;

    // the page that contains the record
    PDBPagePtr myPage, lastPage;


    // how many objects to put into a chunk
    size_t chunkSize = DEFAULT_BATCH_SIZE;

    // where we are in the chunk
    size_t pos = 0;

    // and the tuple set we return
    TupleSetPtr output;

    // size of vector
    size_t mySize = 0;


public:
    // the first param is a callback function that the iterator will call in order to obtain the
    // page holding the next vector to iterate
    // over.  The secomd param is a callback that the iterator will call when the specified page is
    // done being processed and can be
    // freed.  The third param tells us how many objects to put into a tuple set
    PartitionedVectorTupleSetIterator(int myPartitionId,
                           int numPartitionsPerNode,
                           int numNodes,
                           GenericLambdaObjectPtr partitionLambda,
                           std::function<PDBPagePtr()> getAnotherVector,
                           std::function<void(PDBPagePtr)> doneWithVector,
                           size_t chunkSize)
        : getAnotherVector(getAnotherVector), doneWithVector(doneWithVector), chunkSize(chunkSize) {

        this->myPartitionId = myPartitionId;

        this->numPartitionsPerNode = numPartitionsPerNode;

        this->numNodes = numNodes;

        if (partitionLambda == nullptr) {
            std::cout << "Error: partitionLambda doesn't exist while obtaining partitioned source" << std::endl;
            exit(1);
        }
        this->partitionLambda = partitionLambda;

        // create the tuple set that we'll return during iteration
        output = std::make_shared<TupleSet>();
        myPage = nullptr;
        myRec = nullptr;
        // extract the vector from the input page
        myPage = getAnotherVector();
        if (myPage != nullptr) {
           myRec = (Record<Vector<Handle<ValueType>>>*)(myPage->getBytes());
        }
        if (myRec != nullptr) {

            iterateOverMe = myRec->getRootObject();
            this->mySize = iterateOverMe->size();
            std::cout << myPartitionId << ": Got iterateOverMe with " << mySize << " objects" << std::endl;
            // create the output vector and put it into the tuple set
            std::vector<Handle<ValueType>>* inputColumn = new std::vector<Handle<ValueType>>;
            output->addColumn(0, inputColumn, true);
        } else {

            iterateOverMe = nullptr;
            output = nullptr;
        }

        // we are at position zero
        pos = 0;

        // and we have no data so far
        lastRec = nullptr;
        lastPage = nullptr;
    }

    void updatePage () {

        std::cout << myPartitionId << ": to update the page" << std::endl;
        // this means that we got to the end of the vector
        lastPage = myPage;
        if (lastPage != nullptr) {
            doneWithVector(lastPage);
            lastPage = nullptr;
            myRec = nullptr;
        }
        // try to get another vector
        myPage = getAnotherVector();
        if(myPage != nullptr) {
            myRec = (Record<Vector<Handle<ValueType>>>*)(myPage->getBytes());
        }
        // if we could not, then we are outta here
        if (myRec != nullptr) {
            // and reset everything
            iterateOverMe = myRec->getRootObject();
            // JiaNote: we also need to reset mySize
            this->mySize = iterateOverMe->size();
            std::cout << myPartitionId << ": Got iterateOverMe with " << mySize << " objects" << std::endl;
            pos = 0;
        } else {

            iterateOverMe = nullptr;
            mySize = 0;
        }
    }

    void setChunkSize(size_t chunkSize) override {
        this->chunkSize = chunkSize;
    }


    // returns the next tuple set to process, or nullptr if there is not one to process
    TupleSetPtr getNextTupleSet() override {
        // JiaNote: below two lines are necessary to fix a bug that iterateOverMe may be nullptr
        // when first time get to here
        if (iterateOverMe == nullptr) {
            return nullptr;
        }

        if (mySize == 0) {
            return nullptr;
        }

        // see if there are no more items in the vector to iterate over
        if (pos == mySize) {
            updatePage();
        }


        Vector<Handle<ValueType>>& myVec = *iterateOverMe;
        std::vector<Handle<ValueType>>& inputColumn = output->getColumn<Handle<ValueType>>(0);
        // resize the output vector as appropriate
        inputColumn.resize(chunkSize);

        int count = 0;
        size_t hashVal;
        int index;
        while (true) {
            if (pos < mySize) {
                
                hashVal = Hasher<Handle<ValueType>>::hash(myVec[pos]);
                index = hashVal % (numPartitionsPerNode * numNodes) % numPartitionsPerNode;
                if (index == myPartitionId) {
                    if (myPartitionId == 0) {
//                        std::cout << "hashVal: "<< hashVal << ", mySize: "<< mySize <<", index: " << index << ", myPartitionId: " << myPartitionId << ", pos: " << pos <<", count: "<< count <<", chunkSize: "<< chunkSize << std::endl;
                    }
                    inputColumn[count] = myVec[pos];
                    count++;
                    if (count == chunkSize) {
                       pos++;
                       return output;
                    }

                }
                pos++;
                
            } else {

                updatePage();                
                if (mySize == 0) {
                    if (count == 0) return nullptr;
                    inputColumn.resize(count);
                    return output;
                }

                pos = 0;

            }
        }

    }

    ~PartitionedVectorTupleSetIterator() {

        // if lastRec is not a nullptr, then it means that we have not yet freed it
        if ((lastRec != nullptr)||(lastPage != nullptr)) {
            makeObjectAllocatorBlock(4096, true);
            doneWithVector(lastPage);
        }

        lastRec = nullptr;
    }
};
}

#endif
