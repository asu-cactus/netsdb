#ifndef COMBINED_VECTOR_PARTITIONER_H
#define COMBINED_VECTOR_PARTITIONER_H


#include <memory>
#include "Handle.h"
#include "PDBVector.h"
#include "PDBMap.h"
#include "DataTypes.h"

namespace pdb {


class CombinedVectorPartitioner;
typedef std::shared_ptr<CombinedVectorPartitioner> CombinedVectorPartitionerPtr;

class CombinedVectorPartitioner {

private:


    // this is a lambda that we'll call to process input
    std::function<bool(Handle<Object> &)> filterInput1;
    std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> hashInput1;
    std::function<bool(Handle<Object> &)> filterInput2;
    std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> hashInput2;

public:

    // constructor
    CombinedVectorPartitioner(std::function<bool(Handle<Object> &)> filterInput1, 
                                            std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> hashInput1, 
                                            std::function<bool(Handle<Object> &)> filterInput2, 
                                            std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> hashInput2) {

         this->filterInput1 = filterInput1;
         this->hashInput1 = hashInput1;
         this->filterInput2 = filterInput2;
         this->hashInput2 = hashInput2;
    }


    // to run the lambda to transform the input objects
    bool partition(int numNodes, int numPartitions, Handle<Vector<Handle<Object>>> & objectsToProcess, 
                     std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partitionedObjects) {
          for(int i = 0; i < objectsToProcess->size(); i++) {
              Handle<Object> a = (*objectsToProcess)[i];
              if (filterInput1(a)) 
                   (*partitionedObjects)[hashInput1(a, numNodes, numPartitions)]->push_back(a);
              else 
                   (*partitionedObjects)[hashInput2(a, numNodes, numPartitions)]->push_back(a);
          }
          return true;
    }

};

}

#endif

