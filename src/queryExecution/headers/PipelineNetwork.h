#ifndef PIPELINE_NETWORK_H
#define PIPELINE_NETWORK_H

// by Jia, Sept 2016

#include "PipelineNode.h"
#include "DataTypes.h"
#include "PDBLogger.h"
#include "Configuration.h"
#include "SharedMem.h"
#include "JobStage.h"
#include "HermesExecutionServer.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace pdb {

class PipelineNetwork;
typedef std::shared_ptr<PipelineNetwork> PipelineNetworkPtr;


// this class encapsulates the pipeline network
class PipelineNetwork {

private:
    // all source nodes
    std::vector<PipelineNodePtr>* sourceNodes;

    // all nodes in this network
    std::unordered_map<OperatorID, PipelineNodePtr>* allNodes;

    // Job stage id
    Handle<JobStage> jobStage;

    // batch size
    size_t batchSize;

    // number of threads
    int numThreads;

    // node id
    NodeID nodeId;

    // logger
    PDBLoggerPtr logger;

    // configuration
    ConfigurationPtr conf;

    // shared memory
    SharedMemPtr shm;

    // operator id
    OperatorID id;

public:
    // destructor
    ~PipelineNetwork();

    // constructor
    PipelineNetwork(SharedMemPtr shm,
                    PDBLoggerPtr logger,
                    ConfigurationPtr conf,
                    NodeID nodeId,
                    size_t batchSize,
                    int numThreads);

    // initialize from a single job stage
    void initialize(Handle<JobStage> stage);

    // initialize from a graph of job stages
    void initialize(PipelineNodePtr node, Handle<JobStage> stage);

    // return the root job stage corresponding to the pipeline
    Handle<JobStage>& getJobStage();

    // return all source nodes
    std::vector<PipelineNodePtr>* getSourceNodes();

    // append a node to the pipeline network
    bool appendNode(OperatorID parentId, PipelineNodePtr node);

    // append a source node to the pipeline network
    void appendSourceNode(PipelineNodePtr node);

    // return the number of threads that are required to run the pipeline network
    int getNumThreads();

    // return the number of sources of the pipeline network
    int getNumSources();

    // running all sources in the pipeline (in one thread)
    void runAllSources();

    // running the i-the source (in one thread)
    // so you can multiple sources in different threads
    void runSource(int i, HermesExecutionServer* server);
};
}


#endif
