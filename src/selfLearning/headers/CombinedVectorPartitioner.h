#ifndef COMBINED_VECTOR_PARTITIONER_H
#define COMBINED_VECTOR_PARTITIONER_H


#include <memory>
#include "Handle.h"
#include "PDBVector.h"
#include "PDBMap.h"
#include "DataTypes.h"
#include "CombinedVectorPartitionerContext.h"

namespace pdb {


class CombinedVectorPartitioner;
typedef std::shared_ptr<CombinedVectorPartitioner> CombinedVectorPartitionerPtr;

class CombinedVectorPartitioner {

private:


    // this is a lambda that we'll call to process input
    std::vector<std::function<bool(Handle<Object> &)>> filterFuncs;
    std::vector<std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)>> hashFuncs;

public:

    // default constructor
    CombinedVectorPartitioner() {}

    void addContext(CombinedVectorPartitionerContextPtr context) {
          filterFuncs.push_back(context->getFilterFunc());
          hashFuncs.push_back(context->getHashFunc());
    }


    // to run the lambda to transform the input objects
    bool partition(int numNodes, int numPartitions, Handle<Vector<Handle<Object>>> & objectsToProcess, 
                     std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partitionedObjects) {
          for(int i = 0; i < objectsToProcess->size(); i++) {
              Handle<Object> a = (*objectsToProcess)[i];
              if (filterFuncs[0](a)) 
                   (*partitionedObjects)[hashFuncs[0](a, numNodes, numPartitions)]->push_back(a);
              else 
                   (*partitionedObjects)[hashFuncs[1](a, numNodes, numPartitions)]->push_back(a);
          }
          return true;
    }

};

}

#endif

