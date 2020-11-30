#ifndef OBJECTQUERYMODEL_IRPOLICY_CC
#define OBJECTQUERYMODEL_IRPOLICY_CC

#include "PDBDebug.h"
#include "IRPolicy.h"
#include "CombinedVectorPartitioner.h"
#include "CombinedVectorPartitionerContext.h"
#include "Computation.h"
#include "Handle.h"
#include "SetIdentifier.h"

namespace pdb {

IRPolicy::IRPolicy() {
    this->storageNodes = std::vector<NodePartitionDataPtr>();
}

IRPolicy::IRPolicy(int numNodes,
                 int numPartitions,
                 Handle<Vector<Handle<Computation>>> sinks,
                 std::pair<std::string, std::string> source) {

       this->numNodes = numNodes;
       this->numPartitions = numPartitions;
       this->sinks = sinks;
       this->source = source;
       this->storageNodes = std::vector<NodePartitionDataPtr>();
       this->myPolicyName = "Lambda: "+std::to_string(numNodes)+","+std::to_string(numPartitions);
/*
*  computations->push_back(myWriteSet);
  computations->push_back(myWriteSet1);
  computations->push_back(join);
  computations->push_back(join1);
  computations->push_back(select);
  computations->push_back(select1);
  computations->push_back(input1);
  computations->push_back(input2);
  computations->push_back(input3);
*/

    if(sinks->size()!=9){
       std::cout <<"Fatal error: IR policy can only support computation graphs of specific shape" << std::endl;
       exit(1);
    }
    (*sinks)[0]->setInput((*sinks)[2]);
    (*sinks)[2]->setInput(0, (*sinks)[4]);
    (*sinks)[2]->setInput(1, (*sinks)[7]);
    (*sinks)[4]->setInput((*sinks)[6]);
    (*sinks)[1]->setInput(0, (*sinks)[3]);
    (*sinks)[1]->setInput(1, (*sinks)[8]);
    (*sinks)[3]->setInput((*sinks)[6]);
//    this->partitioner = std::make_shared<CombinedVectorPartitioner>();
//    for (int i = 0; i < 2; i++) {
//        CombinedVectorPartitionerContextPtr partitionerContext = (*sinks)[i]->getPartitionerContext(source);
//        partitioner->addContext(partitionerContext);
//    }


}


IRPolicy::~IRPolicy() {}


void IRPolicy::updateStorageNodes(
    Handle<Vector<Handle<NodeDispatcherData>>> activeStorageNodesRaw) {

    auto oldNodes = storageNodes;
    auto activeStorageNodes = createNodePartitionData(activeStorageNodesRaw);
    storageNodes = std::vector<NodePartitionDataPtr>();

    for (int i = 0; i < activeStorageNodes.size(); i++) {
        bool alreadyContains = false;
        for (int j = 0; j < oldNodes.size(); j++) {
            if ((*activeStorageNodes[i]) == (*oldNodes[j])) {
                // Update the pre-existing node with the new information
                auto updatedNode = updateExistingNode(activeStorageNodes[i], oldNodes[j]);
                storageNodes.push_back(updatedNode);
                oldNodes.erase(oldNodes.begin() + j);
                alreadyContains = true;
                break;
            }
        }
        if (!alreadyContains) {
            storageNodes.push_back(updateNewNode(activeStorageNodes[i]));
        }
    }
    for (auto oldNode : oldNodes) {
        handleDeadNode(oldNode);
    }
    this->numNodes = storageNodes.size();
}

std::vector<NodePartitionDataPtr> IRPolicy::createNodePartitionData(
    Handle<Vector<Handle<NodeDispatcherData>>> storageNodes) {
    std::vector<NodePartitionDataPtr> newData = std::vector<NodePartitionDataPtr>();
    for (int i = 0; i < storageNodes->size(); i++) {
        auto nodeData = (*storageNodes)[i];
        auto newNode =
            std::make_shared<NodePartitionData>(nodeData->getNodeId(),
                                                nodeData->getPort(),
                                                nodeData->getAddress(),
                                                std::pair<std::string, std::string>("", ""));
        PDB_COUT << newNode->toString() << std::endl;
        newData.push_back(newNode);
    }
    return newData;
}

NodePartitionDataPtr IRPolicy::updateExistingNode(NodePartitionDataPtr newNode,
                                                          NodePartitionDataPtr oldNode) {
    PDB_COUT << "Updating existing node " << newNode->toString() << std::endl;
    return oldNode;
}

NodePartitionDataPtr IRPolicy::updateNewNode(NodePartitionDataPtr newNode) {
    PDB_COUT << "Updating new node " << newNode->toString() << std::endl;
    return newNode;
}

NodePartitionDataPtr IRPolicy::handleDeadNode(NodePartitionDataPtr deadNode) {
    PDB_COUT << "Deleting node " << deadNode->toString() << std::endl;
    return deadNode;
}

std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>>
IRPolicy::partition(Handle<Vector<Handle<Object>>> toPartition) {
    const UseTemporaryAllocationBlock tempBlock { 256 * 1024 * 1024 };
    std::cout << "IR policy to partition vector of " << toPartition->size() << " elements" << std::endl;
    auto partitionedData =
        std::make_shared<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>>();
    std::cout << numNodes << " nodes" << std::endl;
    for (int i = 0; i < this->numNodes; i++) {
        (*partitionedData)[i] = nullptr;
    }
    if (storageNodes.size() == 0) {
        std::cout
            << "FATAL ERROR: there is no storage node in the cluster, please check conf/serverlist"
            << std::endl;
        exit(-1);
    }
    this->partitioner = std::make_shared<CombinedVectorPartitioner>();
    for (int i = 0; i < 2; i++) {
        CombinedVectorPartitionerContextPtr partitionerContext = (*sinks)[i]->getPartitionerContext(source);
        partitioner->addContext(partitionerContext);
    }
    partitioner->partition(this->numNodes, this->numPartitions, toPartition, partitionedData);
    return partitionedData;
}

}

#endif
