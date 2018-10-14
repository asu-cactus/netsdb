
#ifndef COMPUTE_SINK_H
#define COMPUTE_SINK_H

#include "Object.h"
#include "TupleSet.h"

namespace pdb {

class ComputeSink;
typedef std::shared_ptr<ComputeSink> ComputeSinkPtr;

// this class encapsulates a destination for a set of TupleSet objects.  It may represent
// a hash table that a bunch of objects are being written to, or it may represent secondary
// storage where we are writing the output of a query
class ComputeSink {

public:
    // this creates and returns a new output containter to write to
    virtual Handle<Object> createNewOutputContainer() = 0;

    // this writes the tuple set into the output container
    virtual void writeOut(TupleSetPtr writeMe, Handle<Object>& writeToMe) = 0;

    virtual ~ComputeSink() {}
};
}

#endif
