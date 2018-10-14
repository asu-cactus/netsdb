
#ifndef COMPUTE_SOURCE_H
#define COMPUTE_SOURCE_H

#include "TupleSet.h"

namespace pdb {

class ComputeSource;
typedef std::shared_ptr<ComputeSource> ComputeSourcePtr;

// this class encapsulates some source of TupleSet objects for processing...
// it might wrap up a hash table that we are iterating over, or it might wrap
// up an on-disk set of objects
class ComputeSource {

public:
    // this gets another tuple set for processing
    virtual TupleSetPtr getNextTupleSet() = 0;

    // JiaNote: to enable auto-tuning of batch size in case of failure.
    virtual void setChunkSize(size_t chunkSize) = 0;

    virtual ~ComputeSource() {}
};
}

#endif
