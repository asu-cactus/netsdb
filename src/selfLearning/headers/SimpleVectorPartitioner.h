#ifndef SIMPLE_VECTOR_PARTITIONER_H
#define SIMPLE_VECTOR_PARTITIONER_H


#include <memory>
#include "Handle.h"
#include "PDBVector.h"
#include "PDBMap.h"
#include "DataTypes.h"

namespace pdb {


class SimpleVectorPartitioner;
typedef std::shared_ptr<SimpleVectorPartitioner> SimpleVectorPartitionerPtr;

class SimpleVectorPartitioner {

private:


    // this is a lambda that we'll call to process input
    std::function<bool(int numNodes, int numPartitions, Handle<Vector<Handle<Object>>> &, std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> )> processInput;

public:

    // constructor
    SimpleVectorPartitioner(std::function<bool(int numNodes, int numPartitions, Handle<Vector<Handle<Object>>> &, 
         std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> )> processInput) {

         this->processInput = processInput;
    }


    // to run the lambda to transform the input objects
    bool partition(int numNodes, int numPartitions, Handle<Vector<Handle<Object>>> & objectsToProcess, 
                     std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partitionedObjects) {
          return processInput(numNodes, numPartitions, objectsToProcess, partitionedObjects);
    }

};

}

#endif

