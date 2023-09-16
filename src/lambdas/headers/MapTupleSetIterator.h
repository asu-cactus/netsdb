
#ifndef MAP_TUPLESET_ITER_H
#define MAP_TUPLESET_ITER_H

namespace pdb {

// this class iterates over a pdb :: Map, returning a set of TupleSet objects
template <typename KeyType, typename ValueType, typename OutputType>
class MapTupleSetIterator : public ComputeSource {

private:
    // the map we are iterating over
    Handle<Map<KeyType, ValueType>> iterateOverMe;

    // the number of items to put in each chunk that we produce
    size_t chunkSize;

    // the tuple set we return
    TupleSetPtr output;

    // the iterator for the map
    PDBMapIterator<KeyType, ValueType> begin;
    PDBMapIterator<KeyType, ValueType> end;

public:
    // the first param is a callback function that the iterator will call in order to obtain another
    // vector
    // to iterate over.  The second param tells us how many objects to put into a tuple set
    MapTupleSetIterator(Handle<Object> iterateOverMeIn, size_t chunkSize)
        : iterateOverMe(unsafeCast<Map<KeyType, ValueType>>(iterateOverMeIn)),
          chunkSize(chunkSize),
          begin(iterateOverMe->begin()),
          end(iterateOverMe->end()) {

        output = std::make_shared<TupleSet>();
        std::vector<Handle<OutputType>>* inputColumn = new std::vector<Handle<OutputType>>;
        if (inputColumn == nullptr) {
            std::cout << "MapTupleSetIterator: Failed to allocate memory" << std::endl;
            exit(1);
        }
        output->addColumn(0, inputColumn, true);
    }

    // JiaNote: so that we can tune chunk size automatically in the Pipeline class
    void setChunkSize(size_t chunkSize) override {
        this->chunkSize = chunkSize;
    }
    size_t getChunkSize() override {
        return this->chunkSize;
    }

    // returns the next tuple set to process, or nullptr if there is not one to process
    TupleSetPtr getNextTupleSet() override {

        PDBMapIterator<KeyType, ValueType> beginToRecover = begin;
        PDBMapIterator<KeyType, ValueType> endToRecover = end;
        // see if there are no more items in the vector to iterate over
        if (!(begin != end)) {
            return nullptr;
        }

        std::vector<Handle<OutputType>>& inputColumn = output->getColumn<Handle<OutputType>>(0);
        int limit = inputColumn.size();
        for (int i = 0; i < chunkSize; i++) {
            try {
                if (i >= limit) {
                    Handle<OutputType> temp = (makeObject<OutputType>());
                    inputColumn.push_back(temp);
                } else {
                    inputColumn[i] = (makeObject<OutputType>());
                }
                // key the key/value pair
                inputColumn[i]->getKey() = (*begin).key;
                inputColumn[i]->getValue() = (*begin).value;
            } catch (NotEnoughSpace& n) {
                begin = beginToRecover;
                end = endToRecover;
                inputColumn.clear();
                throw n;
            }
            // move on to the next item
            ++begin;

            // and exit if we are done
            if (!(begin != end)) {
                if (i + 1 < limit) {
                    inputColumn.resize(i + 1);
                }
                return output;
            }
        }

        return output;
    }

    ~MapTupleSetIterator() {}
};
}

#endif
