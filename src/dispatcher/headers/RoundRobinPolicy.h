#ifndef OBJECTQUERYMODEL_ROUNDROBINPOLICY_H
#define OBJECTQUERYMODEL_ROUNDROBINPOLICY_H

// created by Jia, Aug 2017


#include "PartitionPolicy.h"
#include <pthread.h>

namespace pdb {

class RoundRobinPolicy;
typedef std::shared_ptr<RoundRobinPolicy> RoundRobinPolicyPtr;

/**
 * RoundRobinPolicy simply selects the next node from its Storage Nodes List to send the entire
 * Vector of data to. We send
 * the entire Vector to a single node instead of partitioning it on an Object granularity to save
 * time.
 */
class RoundRobinPolicy : public PartitionPolicy {
public:
    RoundRobinPolicy();
    ~RoundRobinPolicy();
    static unsigned int curNodeId;
    int numNodes = 0;
    pthread_mutex_t idMutex;
    void updateStorageNodes(Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);

    std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partition(
        Handle<Vector<Handle<Object>>> toPartition);


private:
    std::vector<NodePartitionDataPtr> createNodePartitionData(
        Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);
    NodePartitionDataPtr updateExistingNode(NodePartitionDataPtr newNodeData,
                                            NodePartitionDataPtr oldNodeData);
    NodePartitionDataPtr updateNewNode(NodePartitionDataPtr newNode);
    NodePartitionDataPtr handleDeadNode(NodePartitionDataPtr deadNode);
};
}


#endif  // OBJECTQUERYMODEL_ROUNDROBINPOLICY_H
