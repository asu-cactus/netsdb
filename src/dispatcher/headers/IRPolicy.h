#ifndef OBJECTQUERYMODEL_IRPOLICY_H
#define OBJECTQUERYMODEL_IRPOLICY_H

// created by Jia, Dec 2020

#include "SetIdentifier.h"
#include "Computation.h"
#include "PartitionPolicy.h"
#include <pthread.h>
#include "CombinedVectorPartitioner.h"
namespace pdb {

class IRPolicy;
typedef std::shared_ptr<IRPolicy> IRPolicyPtr;

/**
 * IRPolicy transforms the input Vector of objects into a map of Vector of objects to NodeID based on the IR of a vector of Computation objects
 * Currently we only support a selection followed by a join and an opposite selection followed by another join
 */
class IRPolicy : public PartitionPolicy {
public:
    //default constructor
    IRPolicy();

    //constructor
    IRPolicy(int numNodes, 
             int numPartitions, 
             Handle<Vector<Handle<Computation>>> sinks,
             std::pair<std::string, std::string> source);


    //destructor
    ~IRPolicy();

    //setters/getters
    void setNumNodes (int numNodes) {
        this->numNodes = numNodes;
    }    

    int getNumNodes () {
        return this->numNodes;
    }

    void setNumPartitions (int numPartitions) {
        this->numPartitions = numPartitions;
    }

    int getNumPartitions () {
        return this->numPartitions;
    }

    void updateStorageNodes(Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);

    std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partition(
        Handle<Vector<Handle<Object>>> toPartition);


private:

    //number of nodes in the cluster
    int numNodes = 0;

    //number of partitions in the cluster
    int numPartitions = 0;

    //the sink of the IR graph
    Handle<Vector<Handle<Computation>>> sinks;

    //the source set, which is to be partitioned, and will later be consumed by the IR graph
    std::pair<string, string> source;

    //partitioner
    CombinedVectorPartitionerPtr partitioner;

    std::vector<NodePartitionDataPtr> createNodePartitionData(
        Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);

    NodePartitionDataPtr updateExistingNode(NodePartitionDataPtr newNodeData,
                                            NodePartitionDataPtr oldNodeData);

    NodePartitionDataPtr updateNewNode(NodePartitionDataPtr newNode);

    NodePartitionDataPtr handleDeadNode(NodePartitionDataPtr deadNode);

};
}


#endif  // OBJECTQUERYMODEL_IRPOLICY_H
