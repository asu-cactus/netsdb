
#ifndef SINK_MERGER_H
#define SINK_MERGER_H



#include "Object.h"
#include "TupleSet.h"


namespace pdb {

class SinkMerger;
typedef std::shared_ptr<SinkMerger> SinkMergerPtr;

// this class encapsulates merger of multiple destinations of a set of TupleSet objects.  It may
// represent
// merging of two hash tables that a bunch of objects are being written to
class SinkMerger {

public:
    // this creates and returns a new output containter to write to
    virtual Handle<Object> createNewOutputContainer() = 0;

    // this writes the tuple set into the output container
    virtual void writeOut(Handle<Object> mergeMe, Handle<Object>& mergeToMe) = 0;

    // this writes the tuple set of multiple maps to the output container
    virtual void writeVectorOut(Handle<Object> mergeMe, Handle<Object>& mergeToMe) = 0;

    // this returns number of hash keys
    virtual int getNumHashKeys() = 0;

    virtual ~SinkMerger() {}
};
}

#endif
