#ifndef TUPLESET_JOBSTAGE_H
#define TUPLESET_JOBSTAGE_H


#include "PDBDebug.h"
#include "Object.h"
#include "DataTypes.h"
#include "Handle.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "SetIdentifier.h"
#include "ComputePlan.h"
#include "AbstractJobStage.h"
#include <stdlib.h>

// PRELOAD %TupleSetJobStage%

namespace pdb {

class TupleSetJobStage : public AbstractJobStage {

public:
    // constructor
    TupleSetJobStage() {}

    // constructor
    TupleSetJobStage(JobStageID stageId) {
        this->id = stageId;
        this->sharedPlan = nullptr;
        this->sourceContext = nullptr;
        this->hashContext = nullptr;
        this->sinkContext = nullptr;
        this->probeOrNot = false;
        this->repartitionOrNot = false;
        this->combineOrNot = false;
        this->broadcastOrNot = false;
        this->inputAggHashOutOrNot = false;
        this->numNodes = 0;
        this->numPartitions = nullptr;
        this->ipAddresses = nullptr;
    }

    // constructor
    TupleSetJobStage(JobStageID stageId, int numNodes) {
        this->id = stageId;
        this->sharedPlan = nullptr;
        this->sourceContext = nullptr;
        this->hashContext = nullptr;
        this->sinkContext = nullptr;
        this->probeOrNot = false;
        this->repartitionOrNot = false;
        this->broadcastOrNot = false;
        this->combineOrNot = false;
        this->inputAggHashOutOrNot = false;
        this->numNodes = numNodes;
        this->numPartitions = makeObject<Vector<Handle<Vector<HashPartitionID>>>>(numNodes);
        this->ipAddresses = makeObject<Vector<String>>(numNodes);
    }

    ~TupleSetJobStage() {}


    std::string getJobStageType() override {
        return "TupleSetJobStage";
    }

    // to set compute plan
    void setComputePlan(Handle<ComputePlan> plan,
                        std::string sourceTupleSetSpecifier,
                        std::string targetTupleSetSpecifier,
                        std::string targetComputationSpecifier) {
        this->sharedPlan = plan;
        this->sourceTupleSetSpecifier = sourceTupleSetSpecifier;
        this->targetTupleSetSpecifier = targetTupleSetSpecifier;
        this->targetComputationSpecifier = targetComputationSpecifier;
    }
    // to get source tupleset name for this pipeline stage
    std::string getSourceTupleSetSpecifier() {
        return this->sourceTupleSetSpecifier;
    }
    // to get target tupleset name for this pipeline stage
    std::string getTargetTupleSetSpecifier() {
        return this->targetTupleSetSpecifier;
    }
    // to get target computation name for this pipeline stage
    std::string getTargetComputationSpecifier() {
        return this->targetComputationSpecifier;
    }

    // to get the vector of tuple sets to build the pipeline
    void getTupleSetsToBuildPipeline(std::vector<std::string>& buildMe) {
        buildMe.clear();
        for (int i = 0; i < (*buildTheseTupleSets).size(); i++) {
            buildMe.push_back((*buildTheseTupleSets)[i]);
        }
    }

    // to set tuplesets for building pipeline
    void setTupleSetsToBuildPipeline(std::vector<std::string> buildMe) {
        buildTheseTupleSets = makeObject<Vector<String>>();
        for (int i = 0; i < buildMe.size(); i++) {
            buildTheseTupleSets->push_back(buildMe[i]);
        }
    }

    //to get tuple sets for building pipeline directly
    Handle<Vector<String>> getTupleSetsToBuildPipeline() {
        return buildTheseTupleSets;
    }

    // nullify compute plan shared pointer
    void nullifyComputePlanPointer() {
        this->sharedPlan->nullifyPlanPointer();
    }


    // to get compute plan
    Handle<ComputePlan> getComputePlan() {
        return this->sharedPlan;
    }

    // to set source set identifier
    void setSourceContext(Handle<SetIdentifier> sourceContext) {
        this->sourceContext = sourceContext;
    }

    // to return source set identifier
    Handle<SetIdentifier> getSourceContext() {
        return this->sourceContext;
    }

    // to set sink set identifier
    void setSinkContext(Handle<SetIdentifier> sinkContext) {
        this->sinkContext = sinkContext;
    }

    // to return sink set identifier
    Handle<SetIdentifier> getSinkContext() {
        return this->sinkContext;
    }

    // to set hash set identifier for probing
    void setHashContext(Handle<SetIdentifier> hashContext) {
        this->hashContext = hashContext;
    }

    // to return hash set identifier for probing
    Handle<SetIdentifier> getHashContext() {
        return this->hashContext;
    }

    // to set combiner set identifier for combining data
    void setCombinerContext(Handle<SetIdentifier> combinerContext) {
        this->combinerContext = combinerContext;
    }

    // to return combiner set identifier for combining data
    Handle<SetIdentifier> getCombinerContext() {
        return this->combinerContext;
    }

    // to set whether to probe hash table
    void setProbing(bool probeOrNot) {
        this->probeOrNot = probeOrNot;
    }

    // to return whether this stage requires to probe hash table
    bool isProbing() {
        return this->probeOrNot;
    }


    // to set whether to probe hash table
    void setInputAggHashOut(bool inputAggHashOutOrNot) {
        this->inputAggHashOutOrNot = inputAggHashOutOrNot;
    }

    // to return whether this stage requires to probe hash table
    bool isInputAggHashOut() {
        return this->inputAggHashOutOrNot;
    }


    // to set whether to broadcast hash table
    void setBroadcasting(bool broadcastOrNot) {
        this->broadcastOrNot = broadcastOrNot;
    }

    // to return whether to broadcast hash table
    bool isBroadcasting() {
        return this->broadcastOrNot;
    }

    // to set whether to repartition the output
    void setRepartition(bool repartitionOrNot) {
        this->repartitionOrNot = repartitionOrNot;
    }

    // to return whether this stage requires to repartition output
    bool isRepartition() {
        return this->repartitionOrNot;
    }


    // to set whether to repartition the output for a join
    void setRepartitionJoin(bool repartitionJoinOrNot) {
        this->repartitionJoinOrNot = repartitionJoinOrNot;
    }

    // to return whether this stage requires to repartition output for a join
    bool isRepartitionJoin() {
        return this->repartitionJoinOrNot;
    }


    // to set whether to combine the repartitioned output
    void setCombining(bool combineOrNot) {
        this->combineOrNot = combineOrNot;
    }

    // to return whether this stage requires to combine repartitioned output
    bool isCombining() {
        return this->combineOrNot;
    }

    std::string getJoinType() {
        return this->joinTypeStr;
    }

    void setJoinType(std::string joinType) {
        this->joinTypeStr = joinType;
    }

    void setJoinType(JoinType joinType) {
   
	if (joinType == HashPartitionedJoin) {
	    this->joinTypeStr = "HashPartitionedJoin";
	} else if (joinType == BroadcastJoin) {
	    this->joinTypeStr = "BroadcastJoin";
	} else if (joinType == LocalJoin) {
	    this->joinTypeStr = "LocalJoin";
	} else if (joinType == CrossProduct) {
	    this->joinTypeStr = "CrossProduct";
	} else {
	    this->joinTypeStr = "UnknownJoin";
	}
    
    }

    JobStageID getStageId() override {
        return this->id;
    }

    void setCollectAsMap(bool collectAsMapOrNot) {
        this->collectAsMapOrNot = collectAsMapOrNot;
    }

    bool isCollectAsMap() {
        return this->collectAsMapOrNot;
    }

    void setJoinTupleSourceOrNot (bool joinTupleSourceOrNot) {
        this->joinTupleSourceOrNot = joinTupleSourceOrNot;
    }

    bool isJoinTupleSource() {
        return this->joinTupleSourceOrNot;
    }

    void setLocalJoinProbeOrNot (bool localJoinProbeOrNot) {
        this->localJoinProbeOrNot = localJoinProbeOrNot;
    }

    bool isLocalJoinProbe() {
        return this->localJoinProbeOrNot;
    }

    void setLocalJoinSinkOrNot (bool localJoinSinkOrNot) {
        this->localJoinSinkOrNot = localJoinSinkOrNot;
    }

    bool isLocalJoinSink() {
        return this->localJoinSinkOrNot;
    }


    int getNumNodesToCollect() {
        return this->numNodesToCollect;
    }

    void setNumNodesToCollect(int numNodesToCollect) {
        this->numNodesToCollect = numNodesToCollect;
    }

    std::string getPartitionComputationSpecifier() {
        return this->partitionComputationSpecifier;
    }

    void setPartitionComputationSpecifier(std::string partitionComputationSpecifier) {
        this->partitionComputationSpecifier = partitionComputationSpecifier;
    }


    std::string getPartitionLambdaName() {
        return this->partitionLambdaName;
    }

    void setPartitionLambdaName (std::string partitionLambdaName) {
        this->partitionLambdaName = partitionLambdaName;
    }

    void print() override {

        std::cout << "[JOB ID] jobId=" << jobId << std::endl;
        std::cout << "[STAGE ID] id=" << id << std::endl;
        std::cout << "[INPUT] databaseName=" << sourceContext->getDatabase()
                  << ", setName=" << sourceContext->getSetName() << std::endl;
        std::cout << "[OUTPUT] databaseName=" << sinkContext->getDatabase()
                  << ", setName=" << sinkContext->getSetName() << std::endl;
        std::cout << "[OUTTYPE] typeName=" << getOutputTypeName() << std::endl;
        std::cout << "[SOURCE TUPLESET] sourceTupleSetSpecifier=" << this->sourceTupleSetSpecifier
                  << std::endl;
        std::cout << "[TARGET TUPLESET] targetTupleSetSpecifier=" << this->targetTupleSetSpecifier
                  << std::endl;
        std::cout << "[TARGET COMPUTATION] targetComputationSpecifier="
                  << this->targetComputationSpecifier << std::endl;
        std::cout << "[ALLOCATOR POLICY] allocatorPolicy=" << this->allocatorPolicy << std::endl;
        std::cout << "[PARTITION COMPUTATION] partitionComputationSpecifier="
                  << this->partitionComputationSpecifier << std::endl;
        std::cout << "[PARTITION LAMBDA] partitionLambdaName="
                  << this->partitionLambdaName << std::endl;
	std::cout << "[JOINTYPE] joinType="
		  << this->joinTypeStr << std::endl;

        if (buildTheseTupleSets != nullptr) {
            std::cout << "[PIPELINE]" << std::endl;
            size_t mySize = buildTheseTupleSets->size();
            for (size_t i = 0; i < mySize; i++) {
                std::cout << i << ": " << (*buildTheseTupleSets)[i] << std::endl;
            }
        }

        std::cout << "[Probing] isProbing=" << this->probeOrNot << std::endl;
        std::cout << "Number of cluster nodes=" << getNumNodes() << std::endl;
        std::cout << "Total memory on this node is " << totalMemoryOnThisNode << std::endl;
        std::cout << "Number of total partitions=" << getNumTotalPartitions() << std::endl;

	int i;
        for (i = 0; i < numNodes; i++) {
            Handle<Vector<HashPartitionID>> partitions = getNumPartitions(i);
            std::cout << "Number of partitions on node-" << i << " is " << partitions->size()
                      << std::endl;
            std::cout << "IP address on node-" << i << " is " << getIPAddress(i) << std::endl;
            int port;
            if ((port = getPort(i)) > 0) {
                std::cout << "Port on node-" << i << " is " << port << std::endl;
            }
        }
        if (repartitionOrNot == true) {
            std::cout << "Repartition the output: true" << std::endl;
        }
        if (localJoinSinkOrNot == true) {
            std::cout << "isLocalJoinSink: true" << std::endl;
        }
        if (localJoinProbeOrNot == true) {
            std::cout << "isLocalJoinProbe: true" << std::endl;
        }

        if (this->probeOrNot == true) {
	    std::cout <<"Hash sets to probe" << std::endl;
	    for (auto a : *(this->hashSetsToProbe)) {
	        std::cout << a.key << ":" << a.value << std::endl;
	    }
        }
    }

    std::string getOutputTypeName() {
        return this->outputTypeName;
    }

    void setOutputTypeName(std::string outputTypeName) {
        this->outputTypeName = outputTypeName;
    }

    void setNumNodes(int numNodes) {
        this->numNodes = numNodes;
    }

    int getNumNodes() {
        return this->numNodes;
    }

    void addNumPartitions(Handle<Vector<HashPartitionID>> numPartitions) {
        this->numPartitions->push_back(numPartitions);
    }

    Handle<Vector<HashPartitionID>>& getNumPartitions(int nodeId) {
        return (*numPartitions)[nodeId];
    }

    Handle<Map<String, String>>& getHashSets() {
        return this->hashSetsToProbe;
    }

    void addHashSetToProbe(String targetTupleSetName, String hashSetName) {
        if (hashSetsToProbe == nullptr) {
            hashSetsToProbe = makeObject<Map<String, String>>();
        }
        (*hashSetsToProbe)[targetTupleSetName] = hashSetName;
    }

    void setHashSetsToProbe(Handle<Map<String, String>> hashSetsToProbe) {
        this->hashSetsToProbe = hashSetsToProbe;
    }

    String getIPAddress(int nodeId) {
        if ((unsigned int)nodeId < numPartitions->size()) {
            std::string ipStr = (*ipAddresses)[nodeId];
            if (ipStr.find(':') != std::string::npos) {
                std::string ip = ipStr.substr(0, ipStr.find(':'));
                return String(ip);
            } else {
                return String(ipStr);
            }
        } else {
            return nullptr;
        }
    }

    int getPort(int nodeId) {
        if ((unsigned int)nodeId < numPartitions->size()) {
            std::string ipStr = (*ipAddresses)[nodeId];
            if (ipStr.find(':') != std::string::npos) {
                int port = atoi((ipStr.substr(ipStr.find(':') + 1)).c_str());
                return port;
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }

    // each address could be simply an IP address like 10.134.96.50, or IP address and port pair
    // like localhost:8109
    void setIPAddresses(Handle<Vector<String>> addresses) {
        this->ipAddresses = addresses;
    }

    void setNumPartitions(Handle<Vector<Handle<Vector<HashPartitionID>>>> numPartitions) {
        this->numPartitions = numPartitions;
    }


    int getNumTotalPartitions() {
        return this->numTotalPartitions;
    }

    void setNumTotalPartitions(int numTotalPartitions) {
        this->numTotalPartitions = numTotalPartitions;
    }

    NodeID getNodeId() {
        return this->myNodeId;
    }

    void setNodeId(NodeID nodeId) {
        this->myNodeId = nodeId;
    }

    bool getNeedsRemoveInputSet() {
        return this->needsRemoveInputSet;
    }

    void setNeedsRemoveInputSet(bool needsRemoveInputSet) {
        this->needsRemoveInputSet = needsRemoveInputSet;
    }

    bool getNeedsRemoveCombinerSet() {
        return this->needsRemoveCombinerSet;
    }

    void setNeedsRemoveCombinerSet(bool needsRemoveCombinerSet) {
        this->needsRemoveCombinerSet = needsRemoveCombinerSet;
    }

    void setTotalMemoryOnThisNode(size_t totalMem) {
        this->totalMemoryOnThisNode = totalMem;
    }

    size_t getTotalMemoryOnThisNode() {
        return this->totalMemoryOnThisNode;
    }

    void setAllocatorPolicy(AllocatorPolicy myPolicy) {
        this->allocatorPolicy = myPolicy;
    }

    AllocatorPolicy getAllocatorPolicy() {
        return this->allocatorPolicy;
    }


    ENABLE_DEEP_COPY


private:
    // Input set information
    Handle<SetIdentifier> sourceContext;

    // Hash set information for probing aggregation set
    Handle<SetIdentifier> hashContext;

    // Combiner set information
    Handle<SetIdentifier> combinerContext;

    // Output set information
    Handle<SetIdentifier> sinkContext;

    // Output type name
    String outputTypeName;

    // logical plan information
    Handle<ComputePlan> sharedPlan;

    // source tuple set
    String sourceTupleSetSpecifier;

    // target tuple set
    String targetTupleSetSpecifier;

    // target computation
    String targetComputationSpecifier;

    // partition computation
    String partitionComputationSpecifier;

    // partition lambda name
    String partitionLambdaName;

    // tuple sets to build the pipeline
    Handle<Vector<String>> buildTheseTupleSets = nullptr;

    // Does this stage has a PartitionedJoinSink
    bool repartitionJoinOrNot = false;


    // Does this stage require probing a hash table ?
    bool probeOrNot = false;

    // Does this stage require repartitioning output?
    bool repartitionOrNot = false;

    // Does this stage require combining repartitioned results?
    bool combineOrNot = false;

    // Does this stage require broadcasting results?
    bool broadcastOrNot = false;

    // Does this stage consume aggregation hash output?
    bool inputAggHashOutOrNot = false;

    // Does this stage collect results to one partition?
    bool collectAsMapOrNot = false;

    // Number of nodes to collect aggregation results
    int numNodesToCollect = 1;

    // hash set names to probe for join
    Handle<Map<String, String>> hashSetsToProbe = nullptr;

    // the id to identify this job stage
    JobStageID id;

    // repartitioning scheme
    int numNodes;

    // number of partitions on each node
    Handle<Vector<Handle<Vector<HashPartitionID>>>> numPartitions = nullptr;

    // IP for each node
    Handle<Vector<String>> ipAddresses;

    // totalPartitions, should be consistent with numPartitions
    int numTotalPartitions;

    // node id of the receiver of this message
    NodeID myNodeId;

    // needs remove combiner set at frontend
    bool needsRemoveCombinerSet;

    // needs remove input set at frontend
    bool needsRemoveInputSet;

    // memory size on this node
    size_t totalMemoryOnThisNode;

    // allocator policy
    AllocatorPolicy allocatorPolicy;

    //Does this stage has a JoinTuple source
    bool joinTupleSourceOrNot = false;
 
    //Does this stage has local join sink
    bool localJoinSinkOrNot = false;

    //Does this stage has local join probe
    bool localJoinProbeOrNot = false;

    String joinTypeStr = "Unknown";

};
}

#endif
