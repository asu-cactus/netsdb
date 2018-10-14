
#ifndef TUPLE_SET_ITER_H
#define TUPLE_SET_ITER_H

namespace pdb {

// this class iterates over an input pdb :: Vector, breaking it up into a series of TupleSet objects
class TupleSetIterator {

private:
    Handle<Vector<Handle<Object>>>& iterateOverMe;
    size_t chunkSize;
    size_t pos;
    TupleSetPtr output;

public:
    TupleSetIterator(Handle<Vector<Handle<Object>>>& iterateOverMe, size_t chunkSize)
        : iterateOverMe(iterateOverMe), chunkSize(chunkSize) {
        output = std::make_shared<TupleSet>();
        std::vector<Handle<Object>>* inputColumn = new std::vector<Handle<Object>>;
        output->addColumn(0, inputColumn, true);
        pos = 0;
    }

    // returns the next tuple set to process, or nullptr if there is not one to process
    TupleSetPtr getNextTupleSet() {
        if (pos == iterateOverMe->size())
            return nullptr;

        int numSlotsToIterate = chunkSize;
        if (numSlotsToIterate + pos > iterateOverMe->size()) {
            numSlotsToIterate = iterateOverMe->size() - pos;
        }

        std::vector<Handle<Object>>& inputColumn = output->getColumn<Handle<Object>>(0);
        inputColumn.resize(numSlotsToIterate);
        for (int i = 0; i < numSlotsToIterate; i++) {
            inputColumn[i] = (*iterateOverMe)[pos];
            pos++;
        }

        return output;
    }
};
}

#endif
