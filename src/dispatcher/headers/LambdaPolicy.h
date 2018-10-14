#ifndef OBJECTQUERYMODEL_LAMBDAPOLICY_H
#define OBJECTQUERYMODEL_LAMBDAPOLICY_H

// created by Jia, Dec 2017


#include "PartitionPolicy.h"
#include "Lambda.h"
#include <pthread.h>

namespace pdb {

class LambdaPolicy;
typedef std::shared_ptr<LambdaPolicy> LambdaPolicyPtr;

/**
 * LambdaPolicy transforms the input Vector of objects into a map of Vector of objects to NodeID.
 */
class LambdaPolicy : public PartitionPolicy {
public:
    //default constructor
    LambdaPolicy();

    //constructor
    LambdaPolicy(int numNodes, 
                 int numPartitions, 
                 GenericLambdaObjectPtr lambda);


    //destructor
    ~LambdaPolicy();

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

    void setLambda (GenericLambdaObjectPtr lambda) {
        this->lambda = lambda;
    }

    GenericLambdaObjectPtr getLambda () {
        return this->lambda;
    }

    void updateStorageNodes(Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);

    std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partition(
        Handle<Vector<Handle<Object>>> toPartition);


private:

    //number of nodes in the cluster
    int numNodes = 0;

    //number of partitions in the cluster
    int numPartitions = 0;

    //the lambda function for partitioning data
    GenericLambdaObjectPtr lambda = nullptr;


    std::vector<NodePartitionDataPtr> createNodePartitionData(
        Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);
    NodePartitionDataPtr updateExistingNode(NodePartitionDataPtr newNodeData,
                                            NodePartitionDataPtr oldNodeData);
    NodePartitionDataPtr updateNewNode(NodePartitionDataPtr newNode);
    NodePartitionDataPtr handleDeadNode(NodePartitionDataPtr deadNode);
};
}


#endif  // OBJECTQUERYMODEL_LAMBDAPOLICY_H
