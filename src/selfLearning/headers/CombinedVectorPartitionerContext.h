#ifndef COMBINED_VECTOR_PARTITIONER_CONTEXT_H
#define COMBINED_VECTOR_PARTITIONER_CONTEXT_H


#include <memory>
#include "Handle.h"
#include "PDBVector.h"
#include "PDBMap.h"
#include "DataTypes.h"

namespace pdb {


class CombinedVectorPartitionerContext;
typedef std::shared_ptr<CombinedVectorPartitionerContext> CombinedVectorPartitionerContextPtr;

class CombinedVectorPartitionerContext {

private:


    // this is a lambda that we'll call to process input
    std::function<bool(Handle<Object> &)> filterFunc;
    std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> hashFunc;

public:

    // constructor
    CombinedVectorPartitionerContext(std::function<bool(Handle<Object> &)> filterFunc, 
                                     std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> hashFunc) {

         this->filterFunc = filterFunc;
         this->hashFunc = hashFunc;
    }

    std::function<bool(Handle<Object> &)> getFilterFunc() {return this->filterFunc;}
    std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> getHashFunc() {return this->hashFunc;}
};

}

#endif

