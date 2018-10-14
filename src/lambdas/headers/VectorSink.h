
#ifndef VECTOR_SINK_H
#define VECTOR_SINK_H

#include "ComputeSink.h"
#include "TupleSetMachine.h"
#include "TupleSet.h"
#include <vector>

namespace pdb {

// writes out all of the items in a tuple set to a vector
template <typename DataType>
class VectorSink : public ComputeSink {

private:
    // the attribute to store
    int whichAttToStore;
    int whichAttToAggregate;

public:
    VectorSink(TupleSpec& inputSchema, TupleSpec& attsToOperateOn) {

        // to setup the output tuple set
        TupleSpec empty;
        TupleSetSetupMachine myMachine(inputSchema, empty);

        // this is the input attribute that we will process
        std::vector<int> matches = myMachine.match(attsToOperateOn);
        whichAttToStore = matches[0];
    }

    Handle<Object> createNewOutputContainer() override {

        // we simply create a new map to store the output
        Handle<Vector<Handle<DataType>>> returnVal = makeObject<Vector<Handle<DataType>>>();
        return returnVal;
    }

    void writeOut(TupleSetPtr input, Handle<Object>& writeToMe) override {

        // get the map we are adding to
        Handle<Vector<Handle<DataType>>> writeMe = unsafeCast<Vector<Handle<DataType>>>(writeToMe);
        auto& myVec = *writeMe;

        // get the input column
        std::vector<Handle<DataType>>& inputColumn =
            input->getColumn<Handle<DataType>>(whichAttToStore);

        // and aggregate everyone
        int length = inputColumn.size();
        for (int i = 0; i < length; i++) {
            try {
                myVec.push_back(inputColumn[i]);
            } catch (NotEnoughSpace& n) {

                // if we got here, we need to erase all of the input that has been processed
                inputColumn.erase(inputColumn.begin(), inputColumn.begin() + i);
                throw n;
            }
        }
    }

    ~VectorSink() {}
};
}

#endif
