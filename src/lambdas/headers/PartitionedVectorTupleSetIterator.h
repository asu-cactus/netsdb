
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

    int loopId = 0;

    GenericLambdaObjectPtr partitionLambda;

    // function to call to get another vector to process
    std::function<PDBPagePtr()> getAnotherVector;

    // function to call to free the vector
    std::function<void(PDBPagePtr)> doneWithVector;

    // this is the vector to process
    Handle<Vector<Handle<Object>>> iterateOverMe;

    // the pointer to the current page holding the vector, and the last page that we previously
    // processed
    Record<Vector<Handle<Object>>>*myRec, *lastRec;

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

    std::vector<Handle<ValueType>>* inputColumn = nullptr;

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
        // create the output vector and put it into the tuple set
        inputColumn = new std::vector<Handle<ValueType>>;
        if (inputColumn == nullptr) {
            std::cout << "CPlusPlusLambda: Failed to allocate memory" << std::endl;
            exit(1);
        }
        output->addColumn(0, inputColumn, true);
        // extract the vector from the input page
        myPage = getAnotherVector();
        if (myPage != nullptr) {
           myRec = (Record<Vector<Handle<Object>>>*)(myPage->getBytes());
        }
        if (myRec != nullptr) {
            iterateOverMe = myRec->getRootObject();
            this->mySize = iterateOverMe->size();
        } else {
            iterateOverMe = nullptr;
        }

        // we are at position zero
        pos = 0;

        // and we have no data so far
        lastRec = nullptr;
        lastPage = nullptr;
    }

    void updatePage () {

        // try to get another vector
        while (mySize == 0) {

            // this means that we got to the end of the vector
            lastPage = myPage;
            if (lastPage != nullptr) {
                doneWithVector(lastPage);
                lastPage = nullptr;
                myRec = nullptr;
            }

            myPage = getAnotherVector();
            if(myPage != nullptr) {
                myRec = (Record<Vector<Handle<Object>>>*)(myPage->getBytes());
            } else {
                iterateOverMe = nullptr;
                return;
            }
            // if we could not, then we are outta here
            if (myRec != nullptr) {
                // and reset everything
                iterateOverMe = myRec->getRootObject();
                // JiaNote: we also need to reset mySize
                this->mySize = iterateOverMe->size();
                pos = 0;
                if (mySize > 0) return;
            } else {

                iterateOverMe = nullptr;
                mySize = 0;
            }
            
        }
    }

    void setChunkSize(size_t chunkSize) override {
        this->chunkSize = chunkSize;
    }
    size_t getChunkSize() override {
        return this->chunkSize;
    }

    // returns the next tuple set to process, or nullptr if there is not one to process
    TupleSetPtr getNextTupleSet() override {




        if (iterateOverMe == nullptr) {
            return nullptr;
        }

        if (mySize == 0) {
            return nullptr;
        }


        // see if there are no more items in the vector to iterate over
        if (pos == mySize) {
            mySize = 0;
            updatePage();
        }

        if (iterateOverMe == nullptr) {
            return nullptr;
        }

        if (mySize == 0) {
            return nullptr;
        }

        Vector<Handle<Object>>& myVec = *iterateOverMe;
        std::vector<Handle<ValueType>>& inputColumn = output->getColumn<Handle<ValueType>>(0);
        // resize the output vector as appropriate
        inputColumn.resize(chunkSize);

        int count = 0;
        size_t hashVal;
        int index;
        while (true) {
            if (pos < mySize) {
                //pageId for debugging
                int pageId = myPage->getPageID();
                int refCount = myPage->getRefCount();
                Handle<ValueType> myIn = unsafeCast<ValueType, Object>(myVec[pos]);
                hashVal = Hasher<ValueType>::hash(*myIn);
                index = hashVal % (numPartitionsPerNode * numNodes) % numPartitionsPerNode;
                if (index == myPartitionId) {
                    inputColumn[count] = myIn;
                    count++;
                    if (count == chunkSize) {
                       pos++;
                       return output;
                    }
                }
                pos++;
                
            } else {
                mySize = 0;
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
    }
};
}

#endif
