#ifndef QUERY_SCHEDULER_SERVER_CC
#define QUERY_SCHEDULER_SERVER_CC

#include "PDBDebug.h"
#include "InterfaceFunctions.h"
#include "QuerySchedulerServer.h"
#include "DistributedStorageManagerClient.h"
#include "DistributedStorageManagerServer.h"
#include "QueryOutput.h"
#include "ResourceInfo.h"
#include "ShuffleInfo.h"
#include "ResourceManagerServer.h"
#include "SimpleSingleTableQueryProcessor.h"
#include "InterfaceFunctions.h"
#include "PDBVector.h"
#include "Handle.h"
#include "ExecuteQuery.h"
#include "TupleSetExecuteQuery.h"
#include "ExecuteComputation.h"
#include "RequestResources.h"
#include "SimpleRequestHandler.h"
#include "SimpleRequestResult.h"
#include "GenericWork.h"
#include "DataTypes.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"
#include "ClusterAggregateComp.h"
#include "QueryGraphAnalyzer.h"
#include "TCAPAnalyzer.h"
#include "Configuration.h"
#include "StorageCollectStats.h"
#include "StorageCollectStatsResponse.h"
#include "Configuration.h"
#include "SelfLearningServer.h"
#include "SelfLearningWrapperServer.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

namespace pdb {

QuerySchedulerServer::~QuerySchedulerServer() {
    pthread_mutex_destroy(&connection_mutex);
}

QuerySchedulerServer::QuerySchedulerServer(PDBLoggerPtr logger,
                                           ConfigurationPtr conf,
                                           bool pseudoClusterMode,
                                           double partitionToCoreRatio,
                                           bool isDynamicPlanning,
                                           bool removeIntermediateDataEarly,
                                           bool selfLearningOrNot) {
    this->port = 8108;
    this->logger = logger;
    this->conf = conf;
    this->pseudoClusterMode = pseudoClusterMode;
    pthread_mutex_init(&connection_mutex, nullptr);
    this->jobStageId = 0;
    this->partitionToCoreRatio = partitionToCoreRatio;
    this->dynamicPlanningOrNot = isDynamicPlanning;
    this->earlyRemovingDataOrNot = removeIntermediateDataEarly;
    this->statsForOptimization = nullptr;
    this->initializeStats();
    this->selfLearningOrNot = selfLearningOrNot;
}


QuerySchedulerServer::QuerySchedulerServer(int port,
                                           PDBLoggerPtr logger,
                                           ConfigurationPtr conf,
                                           bool pseudoClusterMode,
                                           double partitionToCoreRatio,
                                           bool isDynamicPlanning,
                                           bool removeIntermediateDataEarly,
                                           bool selfLearningOrNot) {
    this->port = port;
    this->logger = logger;
    this->conf = conf;
    this->pseudoClusterMode = pseudoClusterMode;
    pthread_mutex_init(&connection_mutex, nullptr);
    this->jobStageId = 0;
    this->partitionToCoreRatio = partitionToCoreRatio;
    this->dynamicPlanningOrNot = isDynamicPlanning;
    this->earlyRemovingDataOrNot = removeIntermediateDataEarly;
    this->statsForOptimization = nullptr;
    this->initializeStats();
    this->selfLearningOrNot = selfLearningOrNot;
}

void QuerySchedulerServer::cleanup() {

    delete this->standardResources;
    this->standardResources = nullptr;

    for (int i = 0; i < queryPlan.size(); i++) {
        queryPlan[i] = nullptr;
    }
    this->queryPlan.clear();

    for (int i = 0; i < interGlobalSets.size(); i++) {
        interGlobalSets[i] = nullptr;
    }
    this->interGlobalSets.clear();

    this->jobStageId = 0;
}

QuerySchedulerServer::QuerySchedulerServer(std::string resourceManagerIp,
                                           int port,
                                           PDBLoggerPtr logger,
                                           ConfigurationPtr conf,
                                           bool usePipelineNetwork,
                                           double partitionToCoreRatio,
                                           bool isDynamicPlanning,
                                           bool removeIntermediateDataEarly,
                                           bool selfLearningOrNot) {

    this->resourceManagerIp = resourceManagerIp;
    this->port = port;
    this->conf = conf;
    this->standardResources = nullptr;
    this->logger = logger;
    this->usePipelineNetwork = usePipelineNetwork;
    this->jobStageId = 0;
    this->partitionToCoreRatio = partitionToCoreRatio;
    this->dynamicPlanningOrNot = isDynamicPlanning;
    this->earlyRemovingDataOrNot = removeIntermediateDataEarly;
    this->statsForOptimization = nullptr;
    this->initializeStats();
    this->selfLearningOrNot = selfLearningOrNot;
}

void QuerySchedulerServer::initialize(bool isRMRunAsServer) {
    if (this->standardResources != nullptr) {
        delete this->standardResources;
    }
    this->standardResources = new std::vector<StandardResourceInfoPtr>();
    if (pseudoClusterMode == false) {
        UseTemporaryAllocationBlock(2 * 1024 * 1024);
        Handle<Vector<Handle<ResourceInfo>>> resourceObjects;
        PDB_COUT << "To get the resource object from the resource manager" << std::endl;
        if (isRMRunAsServer == true) {
            resourceObjects = getFunctionality<ResourceManagerServer>().getAllResources();
        } else {
            ResourceManagerServer rm("conf/serverlist", 8108);
            resourceObjects = rm.getAllResources();
        }

        // add and print out the resources
        for (int i = 0; i < resourceObjects->size(); i++) {

            PDB_COUT << i << ": address=" << (*(resourceObjects))[i]->getAddress()
                     << ", port=" << (*(resourceObjects))[i]->getPort()
                     << ", node=" << (*(resourceObjects))[i]->getNodeId()
                     << ", numCores=" << (*(resourceObjects))[i]->getNumCores()
                     << ", memSize=" << (*(resourceObjects))[i]->getMemSize() << std::endl;
            StandardResourceInfoPtr currentResource = std::make_shared<StandardResourceInfo>(
                (*(resourceObjects))[i]->getNumCores(),
                (*(resourceObjects))[i]->getMemSize(),
                (*(resourceObjects))[i]->getAddress().c_str(),
                (*(resourceObjects))[i]->getPort(),
                (*(resourceObjects))[i]->getNodeId());
            this->standardResources->push_back(currentResource);
        }

    } else {
        UseTemporaryAllocationBlock(2 * 1024 * 1024);
        Handle<Vector<Handle<NodeDispatcherData>>> nodeObjects;
        PDB_COUT << "To get the node object from the resource manager" << std::endl;
        if (isRMRunAsServer == true) {
            nodeObjects = getFunctionality<ResourceManagerServer>().getAllNodes();
        } else {
            ResourceManagerServer rm("conf/serverlist", 8108);
            nodeObjects = rm.getAllNodes();
        }

        // add and print out the resources
        for (int i = 0; i < nodeObjects->size(); i++) {

            PDB_COUT << i << ": address=" << (*(nodeObjects))[i]->getAddress()
                     << ", port=" << (*(nodeObjects))[i]->getPort()
                     << ", node=" << (*(nodeObjects))[i]->getNodeId() << std::endl;
            StandardResourceInfoPtr currentResource =
                std::make_shared<StandardResourceInfo>(DEFAULT_NUM_CORES / (nodeObjects->size()),
                                                       DEFAULT_MEM_SIZE / (nodeObjects->size()),
                                                       (*(nodeObjects))[i]->getAddress().c_str(),
                                                       (*(nodeObjects))[i]->getPort(),
                                                       (*(nodeObjects))[i]->getNodeId());
            this->standardResources->push_back(currentResource);
        }
    }
}

// collect the statistics that will be used for optimizer
// this needs the functionality of catalog and distributed storage manager
void QuerySchedulerServer::initializeStats() {
    // TODO: to load stats from file
    this->statsForOptimization = nullptr;
    this->standardResources = nullptr;
    return;
}

// return statsForOptimization
StatisticsPtr QuerySchedulerServer::getStats() {
    return statsForOptimization;
}

// to schedule dynamic pipeline stages
// this must be invoked after initialize() and before cleanup()
void QuerySchedulerServer::scheduleStages(std::vector<Handle<AbstractJobStage>>& stagesToSchedule,
                                          std::vector<Handle<SetIdentifier>>& intermediateSets,
                                          std::shared_ptr<ShuffleInfo> shuffleInfo, long jobInstanceId) {

    atomic_int counter;
    counter = 0;
    PDBBuzzerPtr tempBuzzer = make_shared<PDBBuzzer>([&](PDBAlarm myAlarm, atomic_int& counter) {
        counter++;
        PDB_COUT << "counter = " << counter << std::endl;
    });

    int numStages = stagesToSchedule.size();

    for (int i = 0; i < numStages; i++) {

        long jobInstanceStageId;
        if (selfLearningOrNot == true) {
            Handle<AbstractJobStage> curStage = stagesToSchedule[i];
            int jobStageId =  curStage->getStageId();
            std::string stageType = curStage->getJobStageType();
            std::cout << "To schedule a job stage of the type " << stageType << std::endl;
            std::string sourceType = "";
            std::string sinkType = "";
            std::string probeType = "";
            Handle<Vector<String>> buildTheseTupleSets = nullptr;
            int numPartitions = shuffleInfo->getNumHashPartitions();
            std::string targetComputationSpecifier = ""; 
            Handle<Computation> aggregationComputation = nullptr;
            if (stageType == "TupleSetJobStage") {
               Handle<TupleSetJobStage> curTupleSetJobStage = 
                   unsafeCast<TupleSetJobStage, AbstractJobStage>(curStage);
               buildTheseTupleSets = curTupleSetJobStage->getTupleSetsToBuildPipeline();
               targetComputationSpecifier = 
                   curTupleSetJobStage->getTargetComputationSpecifier();
               //sourceType
               if (curTupleSetJobStage->isInputAggHashOut()) {
                   sourceType = "Map";
               } else if (curTupleSetJobStage->isJoinTupleSource()) {
                   sourceType = "JoinTuple";
               } else  {
                   sourceType = "Vector";
               }

               //sinkType
               if (curTupleSetJobStage->isBroadcasting()) {
                   sinkType = "Broadcast";
               } else if (curTupleSetJobStage->isRepartition()) {
                   if (curTupleSetJobStage->isRepartitionJoin()) {
                      sinkType = "Repartition";
                   } else {
                      sinkType = "Shuffle";
                   }
               } else {
                   sinkType = "UserSet";
               }

               //probeType
               if (curTupleSetJobStage->isProbing()) {
                   if (curTupleSetJobStage->isJoinTupleSource()) {
                       probeType = "PartitionedHashSet";
                   } else {
                       probeType = "HashSet";
                   }
               } else {
                   probeType = "None";
               }
               

            } else if (stageType == "AggregationJobStage") {
               sourceType = "Vector";
               sinkType = "PartitionedHashSet";
               probeType = "None";
               Handle<AggregationJobStage> curAggregationJobStage =
                   unsafeCast<AggregationJobStage, AbstractJobStage>(curStage);
               aggregationComputation = curAggregationJobStage->getAggComputation();
               if (curAggregationJobStage->needsToMaterializeAggOut()) {
                   sinkType = "UserSet";
               }

            } else if (stageType == "HashPartitionedJoinBuildHTJobStage") {
               sourceType = "Vector";
               sinkType = "PartitionedHashSet";
               probeType = "None";
               Handle<HashPartitionedJoinBuildHTJobStage> curHashPartitionJobStage =
                   unsafeCast<HashPartitionedJoinBuildHTJobStage, AbstractJobStage>
                   (curStage);
               targetComputationSpecifier = 
                   curHashPartitionJobStage->getTargetComputationSpecifier(); 
               buildTheseTupleSets = makeObject<Vector<String>>();
               buildTheseTupleSets->push_back(curHashPartitionJobStage->getSourceTupleSetSpecifier());
               buildTheseTupleSets->push_back(curHashPartitionJobStage->getTargetTupleSetSpecifier());
            } else if (stageType == "BroadcastJoinBuildHTJobStage") {
               sourceType = "Vector";
               sinkType = "BroadcastHashSet";
               probeType = "None";
               Handle<BroadcastJoinBuildHTJobStage> curBroadcastJobStage =
                   unsafeCast<BroadcastJoinBuildHTJobStage, AbstractJobStage>
                   (curStage);
               targetComputationSpecifier =
                   curBroadcastJobStage->getTargetComputationSpecifier();
               buildTheseTupleSets = makeObject<Vector<String>>();
               buildTheseTupleSets->push_back(curBroadcastJobStage->getSourceTupleSetSpecifier());
               buildTheseTupleSets->push_back(curBroadcastJobStage->getTargetTupleSetSpecifier());

            } else {
                std::cout << "Unrecognized JobStage Type: " << stageType << std::endl;
            }
            //create a jobStage entry
            getFunctionality<SelfLearningServer>().createJobStage (jobInstanceId, jobStageId,
                stageType, "Running", sourceType, sinkType, probeType,
                buildTheseTupleSets, numPartitions, targetComputationSpecifier,
                aggregationComputation, jobInstanceStageId);



            //to add data-stage mapping

            if (stageType == "TupleSetJobStage") {
               Handle<TupleSetJobStage> curTupleSetJobStage =
                   unsafeCast<TupleSetJobStage, AbstractJobStage>(curStage);

               //source
               Handle<SetIdentifier> sourceContext =
                   curTupleSetJobStage->getSourceContext();
               //get id of set
               long sourceDataId = getFunctionality<DistributedStorageManagerServer>().
                   getIdForData(sourceContext->getDatabase(), sourceContext->getSetName());
               //add the entry to the DATA_JOB_STAGE
               long sourceMappingId;
               int indexInInputs = sourceContext->getIndexInInputs();
               std::cout << "my input in index is " << indexInInputs << std::endl;
               getFunctionality<SelfLearningServer>().
                   createDataJobStageMapping(sourceDataId, jobInstanceStageId, indexInInputs, "Source", sourceMappingId);
               std::cout << "||||create data job stage mapping: " << sourceDataId << "=>" << jobInstanceStageId << std::endl;
               //sink
               Handle<SetIdentifier> sinkContext =
                   curTupleSetJobStage->getSinkContext();
               //get id of set
               long sinkDataId = getFunctionality<DistributedStorageManagerServer>().
                   getIdForData(sinkContext->getDatabase(), sinkContext->getSetName());
               //add the entry to the DATA_JOB_STAGE
               long sinkMappingId;
               getFunctionality<SelfLearningServer>().
                   createDataJobStageMapping(sinkDataId, jobInstanceStageId, -1, "Sink", sinkMappingId);
               std::cout << "||||create data job stage mapping: " << sinkDataId << "=>" << jobInstanceStageId << std::endl;

               //probe
               if (curTupleSetJobStage->isProbing()) {
                   std::string probeSetType;
                   if (curTupleSetJobStage->isJoinTupleSource()) {
                       probeSetType = "PartitionedHashSet";
                   } else {
                       probeSetType = "HashSet";
                   }
                   Handle<Map<String, String>> & probeSets = curTupleSetJobStage->getHashSets();
                   if (probeSets!= nullptr) {
                       PDBMapIterator<String, String> iter = probeSets->begin();
                       while (iter != probeSets->end()) {
                            String setName = (*iter).value;
                            //add set to DATA if it doesn't exist; and get the id of the set.
                            long curHashDataId = getFunctionality<DistributedStorageManagerServer>().
                                getIdForData("", setName);
                            if (curHashDataId < 0) {
                                getFunctionality<SelfLearningServer>().createData("", setName, curStage->getJobId(),
                                     probeSetType, "IntermediateData", 8191, 0, -1, 1, curHashDataId);
                            } 
                            //add the entry to the DATA_JOB_STAGE
                            long curMappingId;
                            getFunctionality<SelfLearningServer>().
                                createDataJobStageMapping(curHashDataId, jobInstanceStageId, -1, "Probe", curMappingId);
                            ++iter;
                       }
                   }
               }
            } else if  (stageType == "AggregationJobStage") {
               Handle<AggregationJobStage> curAggregationJobStage =
                   unsafeCast<AggregationJobStage, AbstractJobStage>(curStage);

               //source
               Handle<SetIdentifier> sourceContext =
                   curAggregationJobStage->getSourceContext();
               //get id of set
               long sourceDataId = getFunctionality<DistributedStorageManagerServer>().
                   getIdForData(sourceContext->getDatabase(), sourceContext->getSetName());
               //add the entry to the DATA_JOB_STAGE
               long sourceMappingId;
               getFunctionality<SelfLearningServer>().
                   createDataJobStageMapping(sourceDataId, jobInstanceStageId, 0, "Source", sourceMappingId);
               std::cout << "||||create data job stage mapping: " << sourceDataId << "=>" << jobInstanceStageId << std::endl; 
               //sink
               Handle<SetIdentifier> sinkContext =
                   curAggregationJobStage->getSinkContext();
               //get id of set
               long sinkDataId = getFunctionality<DistributedStorageManagerServer>().
                   getIdForData(sinkContext->getDatabase(), sinkContext->getSetName());
               
               if (!curAggregationJobStage->needsToMaterializeAggOut()) {
                   std::string sinkSetType = "PartitionedHashSet";
                   getFunctionality<SelfLearningServer>().createData("", sinkContext->getDatabase() + ":"
                       + sinkContext->getSetName(), curStage->getJobId(), sinkSetType, "IntermediateData", 8191,
                       0, -1, 1, sinkDataId);
               }
               //add the entry to the DATA_JOB_STAGE
               long sinkMappingId;
               getFunctionality<SelfLearningServer>().
                   createDataJobStageMapping(sinkDataId, jobInstanceStageId, -1, "Sink", sinkMappingId);
               std::cout << "||||create data job stage mapping: " << sinkDataId << "=>" << jobInstanceStageId << std::endl; 

            } else if (stageType == "HashPartitionedJoinBuildHTJobStage") {
               Handle<HashPartitionedJoinBuildHTJobStage> curHashPartitionJobStage =
                   unsafeCast<HashPartitionedJoinBuildHTJobStage, AbstractJobStage>
                   (curStage);

               //source
               Handle<SetIdentifier> sourceContext =
                   curHashPartitionJobStage->getSourceContext();
               //get id of set
               long sourceDataId = getFunctionality<DistributedStorageManagerServer>().
                   getIdForData(sourceContext->getDatabase(), sourceContext->getSetName());
               //add the entry to the DATA_JOB_STAGE
               long sourceMappingId;
               getFunctionality<SelfLearningServer>().
                   createDataJobStageMapping(sourceDataId, jobInstanceStageId, -1, "Source", sourceMappingId);
               std::cout << "||||create data job stage mapping: " << sourceDataId << "=>" << jobInstanceStageId << std::endl; 
               //sink
               std::string sinkSetName = curHashPartitionJobStage->getHashSetName();
               //get id of set
               long sinkDataId;
               std::string sinkSetType = "PartitionedHashSet";
               getFunctionality<SelfLearningServer>().createData("", sinkSetName,
                       curStage->getJobId(), sinkSetType, "IntermediateData", 8191,
                       0, -1, 1, sinkDataId);
               
               //add the entry to the DATA_JOB_STAGE
               long sinkMappingId;
               getFunctionality<SelfLearningServer>().
                   createDataJobStageMapping(sinkDataId, jobInstanceStageId, -1, "Sink", sinkMappingId);
               std::cout << "||||create data job stage mapping: " << sinkDataId << "=>" << jobInstanceStageId << std::endl;
            } else if (stageType == "BroadcastJoinBuildHTJobStage") {
               Handle<BroadcastJoinBuildHTJobStage> curBroadcastJobStage =
                   unsafeCast<BroadcastJoinBuildHTJobStage, AbstractJobStage>
                   (curStage);

               //source
               Handle<SetIdentifier> sourceContext =
                   curBroadcastJobStage->getSourceContext();
               //get id of set
               long sourceDataId = getFunctionality<DistributedStorageManagerServer>().
                   getIdForData(sourceContext->getDatabase(), sourceContext->getSetName());
               //add the entry to the DATA_JOB_STAGE
               long sourceMappingId;
               getFunctionality<SelfLearningServer>().
                   createDataJobStageMapping(sourceDataId, jobInstanceStageId, -1, "Source", sourceMappingId);
               std::cout << "||||create data job stage mapping: " << sourceDataId << "=>" << jobInstanceStageId << std::endl; 

               //sink
               std::string sinkSetName = curBroadcastJobStage->getHashSetName();
               //get id of set
               long sinkDataId;
               std::string sinkSetType = "HashSet";
               getFunctionality<SelfLearningServer>().createData("", sinkSetName, 
                       curStage->getJobId(), sinkSetType, "IntermediateData", 8191,
                       0, -1, 1, sinkDataId);

               //add the entry to the DATA_JOB_STAGE
               long sinkMappingId;
               getFunctionality<SelfLearningServer>().
                   createDataJobStageMapping(sinkDataId, jobInstanceStageId, -1, "Sink", sinkMappingId);
               std::cout << "||||create data job stage mapping: " << sinkDataId << "=>" << jobInstanceStageId << std::endl;
            } else {
                std::cout << "Unrecognized JobStage Type: " << stageType << std::endl;
            }

        }


        this->numHashKeys = 0;
        for (int j = 0; j < shuffleInfo->getNumNodes(); j++) {
            PDBWorkerPtr myWorker = getWorker();
            PDBWorkPtr myWork = make_shared<GenericWork>([&, i, j, stagesToSchedule](PDBBuzzerPtr callerBuzzer) {
#ifdef PROFILING
                auto scheduleBegin = std::chrono::high_resolution_clock::now();
#endif


                const UseTemporaryAllocationBlock block(256 * 1024 * 1024);


                int port = (*(this->standardResources))[j]->getPort();
                PDB_COUT << "port:" << port << std::endl;
                std::string ip = (*(this->standardResources))[j]->getAddress();
                PDB_COUT << "ip:" << ip << std::endl;
                size_t memory = (*(this->standardResources))[j]->getMemSize();
                // create PDBCommunicator
                pthread_mutex_lock(&connection_mutex);
                PDB_COUT << "to connect to the remote node" << std::endl;
                PDBCommunicatorPtr communicator = std::make_shared<PDBCommunicator>();

                string errMsg;
                bool success;
                if (communicator->connectToInternetServer(logger, port, ip, errMsg)) {
                    success = false;
                    std::cout << errMsg << std::endl;
                    pthread_mutex_unlock(&connection_mutex);
                    callerBuzzer->buzz(PDBAlarm::GenericError, counter);
                    return;
                }
                pthread_mutex_unlock(&connection_mutex);

                // get current stage to schedule
                Handle<AbstractJobStage> stage = stagesToSchedule[i];

                // schedule the stage
                if (stage->getJobStageType() == "TupleSetJobStage") {
                    Handle<TupleSetJobStage> tupleSetStage =
                        unsafeCast<TupleSetJobStage, AbstractJobStage>(stage);
                    tupleSetStage->setTotalMemoryOnThisNode(memory);
                    success = scheduleStage(j, tupleSetStage, communicator, DeepCopy);
                } else if (stage->getJobStageType() == "AggregationJobStage") {
                    Handle<AggregationJobStage> aggStage =
                        unsafeCast<AggregationJobStage, AbstractJobStage>(stage);
                    int numPartitionsOnThisNode =
                        (int)((double)(standardResources->at(j)->getNumCores()) *
                              partitionToCoreRatio);
                    if (numPartitionsOnThisNode == 0) {
                        numPartitionsOnThisNode = 1;
                    }
                    aggStage->setNumNodePartitions(numPartitionsOnThisNode);
                    aggStage->setAggTotalPartitions(shuffleInfo->getNumHashPartitions());
                    aggStage->setAggBatchSize(DEFAULT_BATCH_SIZE);
                    aggStage->setTotalMemoryOnThisNode(memory);
                    success = scheduleStage(j, aggStage, communicator, DeepCopy);
                } else if (stage->getJobStageType() == "BroadcastJoinBuildHTJobStage") {
                    Handle<BroadcastJoinBuildHTJobStage> broadcastJoinStage =
                        unsafeCast<BroadcastJoinBuildHTJobStage, AbstractJobStage>(stage);
                    broadcastJoinStage->setTotalMemoryOnThisNode(memory);
                    success = scheduleStage(j, broadcastJoinStage, communicator, DeepCopy);
                } else if (stage->getJobStageType() == "HashPartitionedJoinBuildHTJobStage") {
                    Handle<HashPartitionedJoinBuildHTJobStage> hashPartitionedJoinStage =
                        unsafeCast<HashPartitionedJoinBuildHTJobStage, AbstractJobStage>(stage);
                    int numPartitionsOnThisNode =
                        (int)((double)(standardResources->at(j)->getNumCores()) *
                              partitionToCoreRatio);
                    if (numPartitionsOnThisNode == 0) {
                        numPartitionsOnThisNode = 1;
                    }
                    hashPartitionedJoinStage->setNumNodePartitions(numPartitionsOnThisNode);
                    hashPartitionedJoinStage->setTotalMemoryOnThisNode(memory);
                    success = scheduleStage(j, hashPartitionedJoinStage, communicator, DeepCopy);
                } else {
                    errMsg = "Unrecognized job stage";
                    std::cout << errMsg << std::endl;
                    success = false;
                }
#ifdef PROFILING
                auto scheduleEnd = std::chrono::high_resolution_clock::now();
                std::cout << "Time Duration for Scheduling stage-" << stage->getStageId() << " on "
                          << ip << ":"
                          << std::chrono::duration_cast<std::chrono::duration<float>>(scheduleEnd -
                                                                                      scheduleBegin)
                                 .count()
                          << " seconds." << std::endl;
#endif
                if (success == false) {
                    errMsg = std::string("Can't execute the ") + std::to_string(i) +
                        std::string("-th stage on the ") + std::to_string(j) +
                        std::string("-th node");
                    std::cout << errMsg << std::endl;
                    callerBuzzer->buzz(PDBAlarm::GenericError, counter);
                    return;
                }
                callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
            });
            myWorker->execute(myWork, tempBuzzer);
        }
        while (counter < shuffleInfo->getNumNodes()) {
            tempBuzzer->wait();
        }
        counter = 0;
        if (selfLearningOrNot == true) {
              //update the jobStage entry
              getFunctionality<SelfLearningServer>().updateJobStageForCompletion(jobInstanceStageId, "Succeeded");

              std::cout << "****NumHashKeys = " << numHashKeys << std::endl;
              if (numHashKeys > 0) {
                  getFunctionality<SelfLearningServer>().updateJobStageForKeyDistribution(jobInstanceStageId-1, numHashKeys);
              }
        }
    }
}


// JiaNote TODO: consolidate below three functions into a template function
// to replace: schedule(Handle<JobStage>& stage, PDBCommunicatorPtr communicator, ObjectCreationMode
// mode)
bool QuerySchedulerServer::scheduleStage(int index,
                                         Handle<TupleSetJobStage>& stage,
                                         PDBCommunicatorPtr communicator,
                                         ObjectCreationMode mode) {

    bool success;
    std::string errMsg;
    PDB_COUT << "to send the job stage with id=" << stage->getStageId() << " to the " << index
             << "-th remote node" << std::endl;

    if (mode == DeepCopy) {
        const UseTemporaryAllocationBlock block(256 * 1024 * 1024);
        Handle<TupleSetJobStage> stageToSend =
            deepCopyToCurrentAllocationBlock<TupleSetJobStage>(stage);
        stageToSend->setNumNodes(this->shuffleInfo->getNumNodes());
        stageToSend->setNumTotalPartitions(this->shuffleInfo->getNumHashPartitions());
        Handle<Vector<Handle<Vector<HashPartitionID>>>> partitionIds =
            makeObject<Vector<Handle<Vector<HashPartitionID>>>>();
        std::vector<std::vector<HashPartitionID>> standardPartitionIds =
            shuffleInfo->getPartitionIds();
        for (unsigned int i = 0; i < standardPartitionIds.size(); i++) {
            Handle<Vector<HashPartitionID>> nodePartitionIds =
                makeObject<Vector<HashPartitionID>>();
            for (unsigned int j = 0; j < standardPartitionIds[i].size(); j++) {
                nodePartitionIds->push_back(standardPartitionIds[i][j]);
            }
            partitionIds->push_back(nodePartitionIds);
        }
        stageToSend->setNumPartitions(partitionIds);

        Handle<Vector<String>> addresses = makeObject<Vector<String>>();
        std::vector<std::string> standardAddresses = shuffleInfo->getAddresses();
        for (unsigned int i = 0; i < standardAddresses.size(); i++) {
            addresses->push_back(String(standardAddresses[i]));
        }
        stageToSend->setIPAddresses(addresses);
        stageToSend->setNodeId(index);
        success = communicator->sendObject<TupleSetJobStage>(stageToSend, errMsg);
        if (!success) {
            std::cout << errMsg << std::endl;
            return false;
        }
    } else {
        std::cout << "Error: No such object creation mode supported in query scheduler"
                  << std::endl;
        return false;
    }
    PDB_COUT << "to receive query response from the " << index << "-th remote node" << std::endl;
    Handle<SetIdentifier> result = communicator->getNextObject<SetIdentifier>(success, errMsg);
    if (result != nullptr) {
        std::cout << "//////////update stats for TupleSetJobStage" << std::endl; 
        this->updateStats(result);
        PDB_COUT << "TupleSetJobStage execute: wrote set:" << result->getDatabase() << ":"
                 << result->getSetName() << std::endl;
    } else {
        PDB_COUT << "TupleSetJobStage execute failure: can't get results" << std::endl;
        return false;
    }

    return true;
}

bool QuerySchedulerServer::scheduleStage(int index,
                                         Handle<BroadcastJoinBuildHTJobStage>& stage,
                                         PDBCommunicatorPtr communicator,
                                         ObjectCreationMode mode) {
    bool success;
    std::string errMsg;
    PDB_COUT << "to send the job stage with id=" << stage->getStageId() << " to the " << index
             << "-th remote node" << std::endl;

    if (mode == Direct) {
        success = communicator->sendObject<BroadcastJoinBuildHTJobStage>(stage, errMsg);
        if (!success) {
            std::cout << errMsg << std::endl;
            return false;
        }
    } else if (mode == DeepCopy) {
        Handle<BroadcastJoinBuildHTJobStage> stageToSend =
            deepCopyToCurrentAllocationBlock<BroadcastJoinBuildHTJobStage>(stage);
        stageToSend->nullifyComputePlanPointer();
        success = communicator->sendObject<BroadcastJoinBuildHTJobStage>(stageToSend, errMsg);
        if (!success) {
            std::cout << errMsg << std::endl;
            return false;
        }
    } else {
        std::cout << "Error: No such object creation mode supported in query scheduler"
                  << std::endl;
        return false;
    }
    PDB_COUT << "to receive query response from the " << index << "-th remote node" << std::endl;
    Handle<SetIdentifier> result = communicator->getNextObject<SetIdentifier>(success, errMsg);
    if (result != nullptr) {
        this->updateStats(result);
        PDB_COUT << "BroadcastJoinBuildHTJobStage execute: wrote set:" << result->getDatabase()
                 << ":" << result->getSetName() << std::endl;
    } else {
        PDB_COUT << "BroadcastJoinBuildHTJobStage execute failure: can't get results" << std::endl;
        return false;
    }

    return true;
}

bool QuerySchedulerServer::scheduleStage(int index,
                                         Handle<AggregationJobStage>& stage,
                                         PDBCommunicatorPtr communicator,
                                         ObjectCreationMode mode) {
    bool success;
    std::string errMsg;
    PDB_COUT << "to send the job stage with id=" << stage->getStageId() << " to the " << index
             << "-th remote node" << std::endl;

    if (mode == Direct) {
        success = communicator->sendObject<AggregationJobStage>(stage, errMsg);
        if (!success) {
            std::cout << errMsg << std::endl;
            return false;
        }
    } else if (mode == DeepCopy) {
        Handle<AggregationJobStage> stageToSend =
            deepCopyToCurrentAllocationBlock<AggregationJobStage>(stage);
        success = communicator->sendObject<AggregationJobStage>(stageToSend, errMsg);
        if (!success) {
            std::cout << errMsg << std::endl;
            return false;
        }
    } else {
        std::cout << "Error: No such object creation mode supported in query scheduler"
                  << std::endl;
        return false;
    }
    PDB_COUT << "to receive query response from the " << index << "-th remote node" << std::endl;
    Handle<SetIdentifier> result = communicator->getNextObject<SetIdentifier>(success, errMsg);
    if (result != nullptr) {
        this->updateStats(result);
        pthread_mutex_lock(&connection_mutex);
        this->numHashKeys += result->getNumHashKeys();
        std::cout << "***result->getNumHashKeys()=" << result->getNumHashKeys() << std::endl;
        std::cout << "***this->numHashKeys=" << this->numHashKeys << std::endl;
        pthread_mutex_unlock(&connection_mutex);
        PDB_COUT << "AggregationJobStage execute: wrote set:" << result->getDatabase() << ":"
                 << result->getSetName() << std::endl;
    } else {
        PDB_COUT << "AggregationJobStage execute failure: can't get results" << std::endl;
        return false;
    }

    return true;
}

bool QuerySchedulerServer::scheduleStage(int index,
                                         Handle<HashPartitionedJoinBuildHTJobStage>& stage,
                                         PDBCommunicatorPtr communicator,
                                         ObjectCreationMode mode) {
    bool success;
    std::string errMsg;
    PDB_COUT << "to send the job stage with id=" << stage->getStageId() << " to the " << index
             << "-th remote node" << std::endl;

    if (mode == Direct) {
        success = communicator->sendObject<HashPartitionedJoinBuildHTJobStage>(stage, errMsg);
        if (!success) {
            std::cout << errMsg << std::endl;
            return false;
        }
    } else if (mode == DeepCopy) {
        Handle<HashPartitionedJoinBuildHTJobStage> stageToSend =
            deepCopyToCurrentAllocationBlock<HashPartitionedJoinBuildHTJobStage>(stage);
        stageToSend->nullifyComputePlanPointer();
        success = communicator->sendObject<HashPartitionedJoinBuildHTJobStage>(stageToSend, errMsg);
        if (!success) {
            std::cout << errMsg << std::endl;
            return false;
        }
    } else {
        std::cout << "Error: No such object creation mode supported in query scheduler"
                  << std::endl;
        return false;
    }
    PDB_COUT << "to receive query response from the " << index << "-th remote node" << std::endl;
    Handle<SetIdentifier> result = communicator->getNextObject<SetIdentifier>(success, errMsg);
    if (result != nullptr) {
        this->updateStats(result);
        pthread_mutex_lock(&connection_mutex);
        this->numHashKeys += result->getNumHashKeys();
        std::cout << "***result->getNumHashKeys()=" << result->getNumHashKeys() << std::endl;
        std::cout << "***this->numHashKeys=" << this->numHashKeys << std::endl;
        pthread_mutex_unlock(&connection_mutex);
        PDB_COUT << "HashPartitionedJoinBuildHTJobStage execute: wrote set:"
                 << result->getDatabase() << ":" << result->getSetName() << std::endl;
    } else {
        PDB_COUT << "HashPartitionedJoinBuildHTJobStage execute failure: can't get results"
                 << std::endl;
        return false;
    }

    return true;
}




bool QuerySchedulerServer::parseTCAPString(Handle<Vector<Handle<Computation>>> myComputations,
                                           std::string myTCAPString) {
    TCAPAnalyzer tcapAnalyzer(
        this->jobId, myComputations, myTCAPString, this->logger, this->conf, 
        getFunctionality<SelfLearningServer>().getDB(), false);
    return tcapAnalyzer.analyze(this->queryPlan, this->interGlobalSets);
}



// to replace: printCurrentPlan()
void QuerySchedulerServer::printStages() {

    for (int i = 0; i < this->queryPlan.size(); i++) {
        PDB_COUT << "#########The " << i << "-th Plan#############" << std::endl;
        queryPlan[i]->print();
    }
}



// to replace: schedule()
// this must be invoked after initialize() and before cleanup()
void QuerySchedulerServer::scheduleQuery() {

    // query plan
    int numStages = this->queryPlan.size();

    if (numStages > 1) {
        PDB_COUT << "#####################################" << std::endl;
        PDB_COUT << "WARNING: GraphIr generates " << numStages << " stages" << std::endl;
        PDB_COUT << "#####################################" << std::endl;
    }

    std::shared_ptr<ShuffleInfo> shuffleInfo =
        std::make_shared<ShuffleInfo>(this->standardResources, this->partitionToCoreRatio);
    scheduleStages(this->queryPlan, this->interGlobalSets, shuffleInfo);
}



void QuerySchedulerServer::collectStats() {
    this->statsForOptimization = make_shared<Statistics>();
    atomic_int counter;
    counter = 0;
    PDBBuzzerPtr tempBuzzer = make_shared<PDBBuzzer>([&](PDBAlarm myAlarm, atomic_int& counter) {
        counter++;
        PDB_COUT << "counter = " << counter << std::endl;
    });
    if (this->standardResources == nullptr) {
        initialize(true);
    }
    for (int i = 0; i < this->standardResources->size(); i++) {
        PDBWorkerPtr myWorker = getWorker();
        PDBWorkPtr myWork = make_shared<GenericWork>([&, i](PDBBuzzerPtr callerBuzzer) {
            const UseTemporaryAllocationBlock block(4 * 1024 * 1024);

            PDB_COUT << "to collect stats on the " << i << "-th node" << std::endl;
            int port = (*(this->standardResources))[i]->getPort();
            PDB_COUT << "port:" << port << std::endl;
            std::string ip = (*(this->standardResources))[i]->getAddress();
            PDB_COUT << "ip:" << ip << std::endl;

            // create PDBCommunicator
            pthread_mutex_lock(&connection_mutex);
            PDB_COUT << "to connect to the remote node" << std::endl;
            PDBCommunicatorPtr communicator = std::make_shared<PDBCommunicator>();

            string errMsg;
            bool success;
            if (communicator->connectToInternetServer(logger, port, ip, errMsg)) {
                success = false;
                std::cout << errMsg << std::endl;
                pthread_mutex_unlock(&connection_mutex);
                callerBuzzer->buzz(PDBAlarm::GenericError, counter);
                return;
            }
            pthread_mutex_unlock(&connection_mutex);

            // send StorageCollectStats to remote server
            Handle<StorageCollectStats> collectStatsMsg = makeObject<StorageCollectStats>();
            success = communicator->sendObject<StorageCollectStats>(collectStatsMsg, errMsg);
            if (!success) {
                std::cout << errMsg << std::endl;
                callerBuzzer->buzz(PDBAlarm::GenericError, counter);
                return;
            }
            // receive StorageCollectStatsResponse from remote server
            PDB_COUT << "to receive response from the " << i << "-th remote node" << std::endl;
            Handle<StorageCollectStatsResponse> result =
                communicator->getNextObject<StorageCollectStatsResponse>(success, errMsg);
            if (result != nullptr) {
                // update stats
                Handle<Vector<Handle<SetIdentifier>>> stats = result->getStats();
                if (statsForOptimization == nullptr) {
                    statsForOptimization = make_shared<Statistics>();
                }
                for (int j = 0; j < stats->size(); j++) {
                    Handle<SetIdentifier> setToUpdateStats = (*stats)[j];
                    this->updateStats(setToUpdateStats);
                }

            } else {
                errMsg = "Collect response execute failure: can't get results";
                std::cout << errMsg << std::endl;
                callerBuzzer->buzz(PDBAlarm::GenericError, counter);
                return;
            }
            result = nullptr;

            if (success == false) {
                errMsg = std::string("Can't collect stats from node with id=") + std::to_string(i) +
                    std::string(" and ip=") + ip;
                std::cout << errMsg << std::endl;
                callerBuzzer->buzz(PDBAlarm::GenericError, counter);
                return;
            }
            callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
        });
        myWorker->execute(myWork, tempBuzzer);
    }
    while (counter < this->standardResources->size()) {
        tempBuzzer->wait();
    }
    counter = 0;
}

void QuerySchedulerServer::updateStats(Handle<SetIdentifier> setToUpdateStats) {

    std::string databaseName = setToUpdateStats->getDatabase();
    std::string setName = setToUpdateStats->getSetName();
    size_t numPages = setToUpdateStats->getNumPages();
    statsForOptimization->incrementNumPages(databaseName, setName, numPages);
    size_t pageSize = setToUpdateStats->getPageSize();
    statsForOptimization->setPageSize(databaseName, setName, pageSize);
    size_t numBytes = numPages * pageSize;
    statsForOptimization->incrementNumBytes(databaseName, setName, numBytes);
    std::cout << "to increment " << numBytes << " for size" << std::endl;    
}

void QuerySchedulerServer::resetStats(Handle<SetIdentifier> setToResetStats) {

    std::string databaseName = setToResetStats->getDatabase();
    std::string setName = setToResetStats->getSetName();
    statsForOptimization->setNumPages(databaseName, setName, 0);
    statsForOptimization->setPageSize(databaseName, setName, 0);
    statsForOptimization->setNumBytes(databaseName, setName, 0);
}

std::shared_ptr<ShuffleInfo> QuerySchedulerServer::getShuffleInfo () {
    if (this->shuffleInfo == nullptr) {
       initialize(true);
       this->shuffleInfo = std::make_shared<ShuffleInfo>(
             this->standardResources, this->partitionToCoreRatio);
    }
    return this->shuffleInfo;
}


void QuerySchedulerServer::createIntermediateSets(DistributedStorageManagerClient & dsmClient, 
                        std::vector<Handle<SetIdentifier>> & intermediateSets, std::string & errMsg) {

    for (int i = 0; i < intermediateSets.size(); i++) {

         Handle<SetIdentifier> intermediateSet = intermediateSets[i];
         bool res = dsmClient.createTempSet(intermediateSet->getDatabase(),
                           intermediateSet->getSetName(),
                           "IntermediateData",
                           errMsg,
                           intermediateSet->getPageSize(),
                           this->jobId,
                           nullptr,
                           nullptr,
                           intermediateSet->getDesiredSize()
                           /*1*/
                           /*1000*/);
         if (res != true) {
             std::cout << "can't create temp set: " << errMsg << std::endl;
         } else {
             PDB_COUT << "Created set with database="
                      << intermediateSet->getDatabase()
                      << ", set=" << intermediateSet->getSetName()
                      << std::endl;
        }
    }



}


void QuerySchedulerServer::removeIntermediateSets(DistributedStorageManagerClient & dsmClient,
                        std::vector<Handle<SetIdentifier>> & intermediateSets, std::string & errMsg) {

    for (int i = 0; i < intermediateSets.size(); i++) {
        Handle<SetIdentifier> intermediateSet = intermediateSets[i];
        bool res = dsmClient.removeTempSet(intermediateSet->getDatabase(),
                          intermediateSet->getSetName(),
                          "IntermediateData",
                          errMsg);
        if (res != true) {
            std::cout << "can't remove temp set: " << errMsg << std::endl;
        } else {
            PDB_COUT << "Removed set with database="
                     << intermediateSet->getDatabase()
                     << ", set=" << intermediateSet->getSetName()
                     << std::endl;
        }
   }


}




bool QuerySchedulerServer::whetherToMaterialize(Handle<AbstractJobStage> stage) {

    //the rule is that if the source of the stage is a long living set, it should be materialized
    //here, we temporarily hardcode it to always return false;
    //TODO

    //the rule is that if the stage is a hash table building stage, we materialize it
    std::string stageType = stage->getJobStageType();

    if ((stageType == "BroadcastJoinBuildHTJobStage") ||
       (stageType == "HashPartitionedJoinBuildHTJobStage")) {
         std::cout << "Identified a stage to materialize: " << stageType << std::endl;
         return true;

    } else {

         return false;

    }


}


bool QuerySchedulerServer::checkMaterialize(bool materializeThisWorkloadOrNot,
                        std::vector<Handle<SetIdentifier>> & setsToMaterialize,
                        Handle<SetIdentifier> sourceSetIdentifier,
                        std::vector<Handle<AbstractJobStage>> jobStages,
                        std::vector<Handle<AbstractJobStage>> & stagesToMaterialize,
                        std::vector<Handle<SetIdentifier>> intermediateSets,
                        std::vector<Handle<SetIdentifier>> & intermediateSetIdentifierToMaterialize) {


    // stages from the source to a pipeline breaker
    // if the materializeThisWorkloadOrNot is false and the output of the stages should be materialized: 
    //    1.1 we set the output of the last stage to be materialized
    //    1.2 all these stages can be skipped and do not need to be push to the stagesToMaterialize vector
    //    1.3 materializeThisWorkloadOrNot should be set to true
    //    1.4 record the output set that is to be materialized
    // otherwise if materializeThisWorkloadOrNot is false and the output of the stages should NOT be materialized:
    //    2.1 simply push the stages to stagesToMaterialize
    // otherwise if materializeThisWorkloadOrNot is true
    //    3.1 whether the source of these stages consume the materialized set
    //    3.2 if yes, we need set the first stage's source or hashContext to materialized
    //    3.3 if no, we simply push the stages to stagesToMaterialize

    std::cout << "to check whether to materialize this workload" << std::endl;

    bool ret = false;


    std::cout << "we check whether any of the " << jobStages.size() << " stages should trigger a materialization" << std::endl;

    for (int i = 0; i < jobStages.size(); i++) {

        std::cout << "to check the " << i << "-th stage" << std::endl;

        if (whetherToMaterialize(jobStages[i]) == true) {

           Handle<AbstractJobStage> curStage = jobStages[i];

           curStage->setMaterializeOutput(true);
           ret = true;
           Handle<SetIdentifier> sinkSetIdentifier = makeObject<SetIdentifier>();

           std::string stageType = curStage->getJobStageType();
           std::cout << "the stageType is " << stageType << std::endl;
           if (stageType == "TupleSetJobStage") {
               Handle<TupleSetJobStage> castedStage = unsafeCast<TupleSetJobStage>(curStage);
               sinkSetIdentifier = castedStage->getSinkContext();           
           } else if (stageType == "AggregationJobStage") {
               Handle<AggregationJobStage> castedStage = unsafeCast<AggregationJobStage>(curStage);
               sinkSetIdentifier = castedStage->getSinkContext();
           } else if (stageType == "BroadcastJoinBuildHTJobStage") {
               Handle<BroadcastJoinBuildHTJobStage> castedStage = unsafeCast<BroadcastJoinBuildHTJobStage>(curStage);
               sinkSetIdentifier->setDatabase("");
               sinkSetIdentifier->setSetName (castedStage->getHashSetName());
           } else if (stageType == "HashPartitionedJoinBuildHTJobStage") {
               Handle<HashPartitionedJoinBuildHTJobStage> castedStage = unsafeCast<HashPartitionedJoinBuildHTJobStage>(curStage);
               sinkSetIdentifier->setDatabase("");
               sinkSetIdentifier->setSetName (castedStage->getHashSetName());
           } else {
                   //do nothing
           }

           setsToMaterialize.push_back(sinkSetIdentifier);
           break;

       } 
    }

    if (!ret) {
             
       for (int i = 0; i < jobStages.size(); i++) {

           stagesToMaterialize.push_back(jobStages[i]);

       }

       for (int i = 0; i < intermediateSets.size(); i++) {

           intermediateSetIdentifierToMaterialize.push_back(intermediateSets[i]);

       }

    }        


    if ( materializeThisWorkloadOrNot == true ) {

        Handle<Map<String, String>> hashSetsToProbe = nullptr;
        int stageIndex = -1;
        for (int i = 0; i < jobStages.size(); i++) {
        

           Handle<AbstractJobStage> curStage = jobStages[i];
           std::string stageType = curStage->getJobStageType();
           if (stageType == "TupleSetJobStage") {
               Handle<TupleSetJobStage> castedStage = unsafeCast<TupleSetJobStage>(curStage);
               hashSetsToProbe = castedStage->getHashSets();
               stageIndex = i;
               break;
           }

        }
        for (int i = 0; i < setsToMaterialize.size(); i++) {

            Handle<SetIdentifier> curSet = setsToMaterialize[i];
            if ((curSet->getDatabase() == sourceSetIdentifier->getDatabase()) &&
               (curSet->getSetName() == sourceSetIdentifier->getSetName())) {

                jobStages[0]->setSourceMaterialized(true);
                break;

            }

            if (hashSetsToProbe != nullptr) { 

               
               for (PDBMapIterator<String, String> mapIter = hashSetsToProbe->begin();
                   mapIter != hashSetsToProbe->end();
                   ++mapIter) {
                    std::string key = (*mapIter).key;
                    std::string hashSetName = (*mapIter).value;
                    if (curSet->getSetName() == hashSetName) {
                        jobStages[stageIndex]->setHashMaterialized(true);
                        break;
                    }
                }

            }

        }

    }

    return (ret || materializeThisWorkloadOrNot);

}

void QuerySchedulerServer::registerHandlers(PDBServer& forMe) {

    // handler to schedule a Computation-based query graph
    forMe.registerHandler(
        ExecuteComputation_TYPEID,
        make_shared<SimpleRequestHandler<ExecuteComputation>>(

            [&](Handle<ExecuteComputation> request, PDBCommunicatorPtr sendUsingMe) {

                std::string errMsg;
                bool success;

                // parse the query
                const UseTemporaryAllocationBlock block{256 * 1024 * 1024};


                std::cout << "Got the ExecuteComputation object" << std::endl;
                Handle<Vector<Handle<Computation>>> computations =
                    sendUsingMe->getNextObject<Vector<Handle<Computation>>>(success, errMsg);
                std::string tcapString = request->getTCAPString();

                this->jobId = this->getNextJobId();

                long id = -1;
                long instanceId = -1;
                if (this->selfLearningOrNot == true) {
                    std::cout << "To create the Job if not existing" << std::endl;
                    bool ret = getFunctionality<SelfLearningServer>().createJob(request->getJobName(), 
                        tcapString, computations, id);
                    if (ret == true) {
                        size_t numComputations = computations->size();
                        for (size_t i = 0; i < numComputations; i++) {
                            Handle<Computation> curComp = (*computations)[i];
                            std::cout << "check computation: " << curComp->getComputationName() << std::endl;
                            if((curComp->getComputationType() == "JoinComp")||(curComp->getComputationType() == "ClusterAggregationComp")) {
                                std::cout << "to populate lambdas" << std::endl;
                                curComp->populateLambdas(id, getFunctionality<SelfLearningWrapperServer>());
                            }
                        } 
                    } 
                    getFunctionality<SelfLearningServer>().createJobInstance (id, 
                        this->jobId, instanceId);
                    
                }

                DistributedStorageManagerClient dsmClient(this->port, "localhost", logger);

                // create the database first
                success = dsmClient.createDatabase(this->jobId, errMsg);


                if ((request->getWhetherToPreCompile() == true) && (materializedWorkloads.count(request->getJobName()))) {


                    std::cout << "We find precompiled stages for " << request->getJobName() << std::endl;
                    getFunctionality<QuerySchedulerServer>().initialize(true);
                    this->shuffleInfo = std::make_shared<ShuffleInfo>(
                            this->standardResources, this->partitionToCoreRatio);


                    PreCompiledWorkloadPtr workload = materializedWorkloads[request->getJobName()];
                    workload->print();
                    std::vector<Handle<AbstractJobStage>> jobStages = workload->getStages();
                    std::vector<Handle<SetIdentifier>> intermediateSets = workload->getIntermediateSets();

                    std::string errMsg;
                    createIntermediateSets(dsmClient, intermediateSets, errMsg);
                    // schedule this job stages
                    PDB_COUT << "To schedule the query to run on the cluster" << std::endl;
                    getFunctionality<QuerySchedulerServer>().scheduleStages(
                                jobStages, intermediateSets, shuffleInfo, instanceId);
                    removeIntermediateSets(dsmClient, intermediateSets, errMsg);

                }
                else if (success == true) {
                    // we do not use dynamic planning
                    if (this->dynamicPlanningOrNot == false) {
                        success = parseTCAPString(computations, tcapString);
                        if (success == false) {
                            errMsg =
                                "FATAL ERROR in QuerySchedulerServer: can't parse TCAP string.\n" +
                                tcapString;
                        } else {
                            // create intermediate sets:
                            PDB_COUT << "to create intermediate sets" << std::endl;
                            if (success == true) {
                                std::string errMsg;
                                createIntermediateSets(dsmClient, this->interGlobalSets, errMsg);
                                getFunctionality<QuerySchedulerServer>().printStages();
                                PDB_COUT << "To get the resource object from the resource manager"
                                         << std::endl;
                                getFunctionality<QuerySchedulerServer>().initialize(true);
                                PDB_COUT << "To schedule the query to run on the cluster"
                                         << std::endl;
                                getFunctionality<QuerySchedulerServer>().scheduleQuery();

                                PDB_COUT << "to remove intermediate sets" << std::endl;
                                removeIntermediateSets(dsmClient, this->interGlobalSets, errMsg);
                            }
                        }

                    } else {

                        // analyze resources
                       
                        PDB_COUT << "To get the resource object from the resource manager"
                                 << std::endl;
                        getFunctionality<QuerySchedulerServer>().initialize(true);
                        this->shuffleInfo = std::make_shared<ShuffleInfo>(
                            this->standardResources, this->partitionToCoreRatio);

                        if (this->statsForOptimization == nullptr) {
                            this->collectStats();
                        }


                        // dyanmic planning
                        // initialize tcapAnalyzer
                        this->tcapAnalyzerPtr = make_shared<TCAPAnalyzer>(
                            jobId, computations, tcapString, this->logger, this->conf, 
                            getFunctionality<SelfLearningServer>().getDB(), true);
                        this->tcapAnalyzerPtr->setNumNodes(this->standardResources->size());
                        int jobStageId = 0;

                        // this is to specify whether we should materialize all stages regarding the current workload
                        // in order to save time for future executions
                        bool materializeThisWorkloadOrNot = false;


                        // a sequence of stages to be materialized for this workload
                        // at the end, if materializeThisWorkloadOrNot = false, we need clear this vector,
                        // otherwise, we need add all these stages to a PreCompiledWorkload instance, and add this instance to the hashmap
                        std::vector<Handle<AbstractJobStage>> stagesToMaterialize;

                        // intermediate set identifiers to materialize together with stages
                        std::vector<Handle<SetIdentifier>> setIdentifiersToMaterialize;

                        // materialized sets
                        std::vector<Handle<SetIdentifier>> setsToMaterialize;


                        while (this->tcapAnalyzerPtr->getNumSources() > 0) {
                            std::vector<Handle<AbstractJobStage>> jobStages;
                            std::vector<Handle<SetIdentifier>> intermediateSets;
                            Handle<SetIdentifier> sourceSet;
#ifdef PROFILING
                            auto dynamicPlanBegin = std::chrono::high_resolution_clock::now();
                            std::cout << "JobStageId " << jobStageId << "============>";
#endif
                            while ((jobStages.size() == 0) &&
                                   (this->tcapAnalyzerPtr->getNumSources() > 0)) {

                                // analyze all sources and select a source based on cost model
                                int indexOfBestSource = this->tcapAnalyzerPtr->getBestSource(
                                    this->statsForOptimization);

                                // get the job stages and intermediate data sets for this source
                                std::string sourceName =
                                    this->tcapAnalyzerPtr->getSourceSetName(indexOfBestSource);
                                std::cout << "best source is " << sourceName << std::endl;
                                sourceSet =
                                    this->tcapAnalyzerPtr->getSourceSetIdentifier(sourceName);
                                AtomicComputationPtr sourceAtomicComp =
                                    this->tcapAnalyzerPtr->getSourceComputation(sourceName);
                                unsigned int sourceConsumerIndex =
                                    this->tcapAnalyzerPtr->getNextConsumerIndex(sourceName);
                                bool hasConsumers = this->tcapAnalyzerPtr->getNextStagesOptimized(
                                    jobStages,
                                    intermediateSets,
                                    sourceAtomicComp,
                                    sourceSet,
                                    sourceConsumerIndex,
                                    jobStageId);
                                std::cout << "jobStages have " << jobStages.size() << " stages" << std::endl;
                                if (jobStages.size() > 0) {
                                    this->tcapAnalyzerPtr->incrementConsumerIndex(sourceName);
                                    break;
                                } else {
                                    if (hasConsumers == false) {
                                        std::cout << "we didn't meet a penalized set and we remove "
                                                     "source "
                                                  << sourceName << std::endl;
                                        this->tcapAnalyzerPtr->removeSource(sourceName);
                                    }
                                }

                            }
                            
                            this->statsForOptimization->clearPenalizedCosts();



                            // stages from the source to a pipeline breaker
                            // if the materializeThisWorkloadOrNot is false and the output of the stages should be materialized: 
                            //    1.1 we set the output of the last stage to be materialized
                            //    1.2 all these stages can be skipped and do not need to be push to the stagesToMaterialize vector
                            //    1.3 materializeThisWorkloadOrNot should be set to true
                            //    1.4 record the output set that is to be materialized
                            // otherwise if materializeThisWorkloadOrNot is false and the output of the stages should NOT be materialized:
                            //    2.1 simply push the stages to stagesToMaterialize
                            // otherwise if materializeThisWorkloadOrNot is true
                            //    3.1 whether the source of these stages consume the materialized set
                            //    3.2 if yes, we need set the first stage's source or hashContext to materialized
                            //    3.3 if no, we simply push the stages to stagesToMaterialize

                            if (request->getWhetherToPreCompile() == true) {
                                std::cout << "invoking checkMaterialize" << std::endl;
                                bool ret = checkMaterialize(materializeThisWorkloadOrNot, setsToMaterialize, sourceSet,
                                                 jobStages, stagesToMaterialize,
                                                 intermediateSets, setIdentifiersToMaterialize);
                                materializeThisWorkloadOrNot = ret;
                                if (ret) {
                                    std::cout << "We need to materialize the workload" << std::endl;
                                    std::cout << "there are " << stagesToMaterialize.size() << " stages to materialize" << std::endl;
                                    std::cout << "there are " << setIdentifiersToMaterialize.size() << " intermediate set identifiers to materialize" << std::endl;
                                    std::cout << "there are " << setsToMaterialize.size() << " sets to materialize" << std::endl;
                                }
                            }
                            


#ifdef PROFILING
                            auto dynamicPlanEnd = std::chrono::high_resolution_clock::now();
                            std::cout << "Time Duration for Dynamic Planning: "
                                      << std::chrono::duration_cast<std::chrono::duration<float>>(
                                             dynamicPlanEnd - dynamicPlanBegin)
                                             .count()
                                      << " seconds." << std::endl;
                            auto createSetBegin = std::chrono::high_resolution_clock::now();
#endif
                            // create intermediate sets
                            std::string errMsg;
                            createIntermediateSets(dsmClient,intermediateSets, errMsg);

#ifdef PROFILING
                            auto createSetEnd = std::chrono::high_resolution_clock::now();
                            std::cout << "Time Duration for Creating intermdiate sets: "
                                      << std::chrono::duration_cast<std::chrono::duration<float>>(
                                             createSetEnd - createSetBegin)
                                             .count()
                                      << " seconds." << std::endl;
                            auto scheduleBegin = std::chrono::high_resolution_clock::now();
#endif
                            // schedule this job stages
                            PDB_COUT << "To schedule the query to run on the cluster" << std::endl;
                            getFunctionality<QuerySchedulerServer>().scheduleStages(
                                jobStages, intermediateSets, shuffleInfo, instanceId);
  
#ifdef PROFILING
                            auto scheduleEnd = std::chrono::high_resolution_clock::now();
                            std::cout << "Time Duration for Scheduling stages: "
                                      << std::chrono::duration_cast<std::chrono::duration<float>>(
                                             scheduleEnd - scheduleBegin)
                                             .count()
                                      << " seonds." << std::endl;
                            auto removeSetBegin = std::chrono::high_resolution_clock::now();
#endif




                            // to remove the intermediate sets:
                            for (int i = 0; i < intermediateSets.size(); i++) {
                                std::string errMsg;
                                Handle<SetIdentifier> intermediateSet = intermediateSets[i];
                                // check whether intermediateSet is a source set and has consumer
                                // number > 0
                                std::string key = intermediateSet->getDatabase() + ":" +
                                    intermediateSet->getSetName();
                                unsigned int numConsumers =
                                    this->tcapAnalyzerPtr->getNumConsumers(key);
                                if (numConsumers > 0) {
                                    // to remember this set
                                    this->interGlobalSets.push_back(intermediateSet);

                                } else {

                                    if (selfLearningOrNot == true) {

                                        // to get the id of the set
                                        long id = getFunctionality<DistributedStorageManagerServer>().getIdForData(
                                                    intermediateSet->getDatabase(), intermediateSet->getSetName());
                                        std::cout <<"///////////id for " << intermediateSet->getDatabase() << ":" << intermediateSet->getSetName() 
                                                  <<" is " << id << std::endl;

                                        // to get the size of the set
                                        size_t size = this->statsForOptimization->getNumBytes(
                                                    intermediateSet->getDatabase(), intermediateSet->getSetName());
                                    
                                        // update the size of the set
                                        getFunctionality<SelfLearningServer>().updateDataForSize(id, size);
                                        std::cout <<"///////////to update data with id=" << id << " for size=" << size << std::endl;
                                    }

                                    bool res =
                                        dsmClient.removeTempSet(intermediateSet->getDatabase(),
                                                                intermediateSet->getSetName(),
                                                                "IntermediateData",
                                                                errMsg);
                                    if (res != true) {
                                        std::cout << "can't remove temp set: " << errMsg
                                                  << std::endl;
                                    } else {
                                        std::cout << "Removed set with database="
                                                  << intermediateSet->getDatabase()
                                                  << ", set=" << intermediateSet->getSetName()
                                                  << std::endl;
                                    }
                                }
                            }
#ifdef PROFILING
                            auto removeSetEnd = std::chrono::high_resolution_clock::now();
                            std::cout << "Time Duration for Removing intermediate sets: "
                                      << std::chrono::duration_cast<std::chrono::duration<float>>(
                                             removeSetEnd - removeSetBegin)
                                             .count()
                                      << " seconds." << std::endl;
#endif
                        }//while
                        // to remove remaining intermediate sets:
                        std::cout << "to remove remaining intermediate sets" << std::endl;

                        removeIntermediateSets(dsmClient, this->interGlobalSets, errMsg);

                        // create a PreCompiledWorkload from materializeThisWorkloadOrNot, stagesToMaterialize, intermediateSetsToMaterialize
                        // insert the PreCompiledWorkload to a hashmap

                        if (request->getWhetherToPreCompile() == true) {
                            std::cout << "to create precompiled stages" << std::endl;
                            PreCompiledWorkloadPtr workload = std::make_shared<PreCompiledWorkload>(stagesToMaterialize, setIdentifiersToMaterialize);
                            workload->print();
                            materializedWorkloads[request->getJobName()] = workload;
                        }

                    }//if dynamic scheduling is false
                }//if database is successfully created
                if (selfLearningOrNot == true) {
                    std::string status;
                    if (success == true) {
                         status = "Succeeded";
                    } else {
                         status = "Failed";
                    }
                    getFunctionality<SelfLearningServer>().updateJobInstanceForCompletion (instanceId, status);
                }
                PDB_COUT << "To send back response to client" << std::endl;
                Handle<SimpleRequestResult> result =
                    makeObject<SimpleRequestResult>(success, errMsg);
                if (!sendUsingMe->sendObject(result, errMsg)) {
                    PDB_COUT << "to cleanup" << std::endl;
                    getFunctionality<QuerySchedulerServer>().cleanup();
                    return std::make_pair(false, errMsg);
                }
                PDB_COUT << "to cleanup" << std::endl;
                getFunctionality<QuerySchedulerServer>().cleanup();
                return std::make_pair(true, errMsg);
            }

            ));


}
}


#endif
