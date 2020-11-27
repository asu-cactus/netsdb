#ifndef SIMPLE_PARTITIONER_H
#define SIMPLE_PARTITIONER_H


#include <memory>
#include "Handle.h"
#include "PDBVector.h"
#include "PDBMap.h"
#include "DataTypes.h"

namespace pdb {


class SimplePartitioner;
typedef std::shared_ptr<SimplePartitioner> SimplePartitionerPtr;

class SimplePartitioner {

private:


    // this is a lambda that we'll call to process input
    std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> processInput;

public:

    // constructor
    SimplePartitioner(std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> processInput) {

         this->processInput = processInput;
    }

    std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)>& getPartitioner() {
         return this->processInput;
    }

};

}

#endif

