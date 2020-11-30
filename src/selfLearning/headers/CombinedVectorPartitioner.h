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
          std::cout << "filterFuncs.size()="<<filterFuncs.size()<<std::endl;
          std::cout << "hashFuncs.size()="<<hashFuncs.size()<<std::endl;
    }


    // to run the lambda to transform the input objects
    bool partition(int numNodes, int numPartitions, Handle<Vector<Handle<Object>>> & objectsToProcess, 
                     std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partitionedObjects) {
          for(int i = 0; i < objectsToProcess->size(); i++) {
              Handle<Object> a = (*objectsToProcess)[i];
              NodeID nodeId;
              bool result = filterFuncs[0](a);
              if (result) {
                  std::cout <<"we should apply the first hash function" << std::endl;
                  nodeId = hashFuncs[0](a, numNodes, numPartitions);
              } else{
                  std::cout <<"we should apply the second hash function" << std::endl;
                  nodeId = hashFuncs[1](a, numNodes, numPartitions);
              }
              std::cout <<"this object should be dispatched to the node with id="<<nodeId<<std::endl;
              Handle<Vector<Handle<Object>>> myVec = (*partitionedObjects)[nodeId];
              if(myVec == nullptr) {
                   std::cout <<"to create a new vector" << std::endl;
                   myVec = makeObject<Vector<Handle<Object>>>();
                   (*partitionedObjects)[nodeId] = myVec;
              } 
              std::cout <<"to push back the vector" << std::endl;
              (*partitionedObjects)[nodeId]->push_back(a);  
              std::cout <<"now the list size is " << (*partitionedObjects)[nodeId]->size()<<std::endl;
          }
          return true;
    }

};

}

#endif

