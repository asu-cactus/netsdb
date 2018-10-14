
#ifndef SINK_SHUFFLER_H
#define SINK_SHUFFLER_H



#include "Object.h"
#include "TupleSet.h"
#include "DataTypes.h"


namespace pdb {

class SinkShuffler;
typedef std::shared_ptr<SinkShuffler> SinkShufflerPtr;

// this class encapsulates a shuffler to pick up join maps in the input pages that belongs to the
// shuffler's destination node and create them again in a set of output pages. So the set of output
// pages are belonging to the same destination node.
class SinkShuffler {

public:
    // this sets the id for the destination node
    virtual void setNodeId(int nodeId) = 0;

    // this gets id of the destination node
    virtual int getNodeId() = 0;

    // this creates and returns a new output containter to write to
    virtual Handle<Object> createNewOutputContainer() = 0;

    // this writes the tuple set into the output container
    virtual bool writeOut(Handle<Object> shuffleMe, Handle<Object>& shuffleToMe) = 0;

    virtual ~SinkShuffler() {}
};
}

#endif
