
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
    size_t pos;

    // and the tuple set we return
    TupleSetPtr output;

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
            PDB_COUT << "Got iterateOverMe" << std::endl;
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

        // if we made it here with lastRec being a valid pointer, then it means
        // that we have gone through an entire cycle, and so all of the data that
        // we will ever reference stored in lastRec has been fluhhed through the
        // pipeline; hence, we can kill it

        if (lastRec != nullptr) {
            doneWithVector(lastPage);
            lastRec = nullptr;
            lastPage = nullptr;
        }

        size_t mySize = iterateOverMe->size();
        if (mySize == 0) {
            return nullptr;
        }

        // see if there are no more items in the vector to iterate over
        if (pos == mySize) {
            // this means that we got to the end of the vector
            lastPage = myPage;
            lastRec = myRec;
            myPage = nullptr;
            myRec = nullptr;
            // try to get another vector
            myPage = getAnotherVector();
            if(myPage != nullptr) {
                myRec = (Record<Vector<Handle<ValueType>>>*)(myPage->getBytes());
            }
            // if we could not, then we are outta here
            if (myRec == nullptr)
                return nullptr;

            // and reset everything
            iterateOverMe = myRec->getRootObject();
            // JiaNote: we also need to reset mySize
            mySize = iterateOverMe->size();

            if (mySize == 0) {
                return nullptr;
            }

            pos = 0;
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
                    inputColumn[count] = myVec[pos];
                    count++;
                    if (count == chunkSize) {
                       pos++;
                       return output;
                    }

                }
                pos++;
                
            } else {

                // this means that we got to the end of the vector
                lastRec = myRec;
                lastPage = myPage;
                myPage = nullptr;
                myRec = nullptr;
                // try to get another vector
                myPage = getAnotherVector();
                if (myPage != nullptr) {
                    myRec = (Record<Vector<Handle<ValueType>>>*)(myPage->getBytes());
                }
                // if we could not, then we are outta here
                if (myRec == nullptr) {
                    inputColumn.resize(count);
                    return output;
                }
                // and reset everything
                iterateOverMe = myRec->getRootObject();
                // JiaNote: we also need to reset mySize
                mySize = iterateOverMe->size();

                if (mySize == 0) {
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
