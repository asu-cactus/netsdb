
#ifndef TCAP_ANALYZER_CC
#define TCAP_ANALYZER_CC

#include "TCAPAnalyzer.h"
#include "JoinComp.h"
#include "MultiSelectionComp.h"
#include "ScanUserSet.h"
#include "SelectionComp.h"
#include "PartitionComp.h"
#include <cfloat>

#ifndef JOIN_COST_THRESHOLD
#define JOIN_COST_THRESHOLD 15000
#endif
namespace pdb {

TCAPAnalyzer::TCAPAnalyzer(std::string jobId,
                           Handle<Vector<Handle<Computation>>> myComputations,
                           std::string myTCAPString, PDBLoggerPtr logger,
                           ConfigurationPtr conf, std::shared_ptr<SelfLearningDB> db,
                           bool isDynamicPlanning) {
  this->jobId = jobId;
  this->computations = myComputations;
  this->tcapString = myTCAPString;
  this->logger = logger;
  this->conf = conf;
  this->db = db;
  try {
    this->computePlan =
        makeObject<ComputePlan>(String(myTCAPString), *myComputations);
    this->logicalPlan = this->computePlan->getPlan();
    this->computePlan->nullifyPlanPointer();
    this->computationGraph = this->logicalPlan->getComputations();
    this->sources = this->computationGraph.getAllScanSets();
  } catch (pdb::NotEnoughSpace &n) {
    std::cout << "FATAL ERROR in TCAPAnalyzer: Not enough memory to allocate "
                 "the computePlan object";
    logger->fatal("FATAL ERROR in TCAPAnalyzer: Not enough memory to allocate "
                  "the computePlan object");
    this->computePlan = nullptr;
    this->logicalPlan = nullptr;
    this->sources.clear();
  }
  hashSetsToProbe = nullptr;
  this->dynamicPlanningOrNot = isDynamicPlanning;
  if (this->dynamicPlanningOrNot == true) {
    // initialize source sets and source nodes;
    for (int i = 0; i < this->sources.size(); i++) {
      AtomicComputationPtr curSource = sources[i];
      Handle<SetIdentifier> curInputSetIdentifier = nullptr;
      std::string sourceSpecifier = curSource->getComputationName();
      Handle<Computation> sourceComputation =
          this->logicalPlan->getNode(sourceSpecifier).getComputationHandle();
      if ((sourceComputation->getComputationType() == "ScanUserSet") ||
          (sourceComputation->getComputationType() == "ScanSet")) {
        Handle<ScanUserSet<Object>> scanner =
            unsafeCast<ScanUserSet<Object>, Computation>(sourceComputation);
        curInputSetIdentifier = makeObject<SetIdentifier>(
            scanner->getDatabaseName(), scanner->getSetName());
      } else if (sourceComputation->getComputationType() ==
                 "ClusterAggregationComp") {
        Handle<AbstractAggregateComp> aggregator =
            unsafeCast<AbstractAggregateComp, Computation>(sourceComputation);
        curInputSetIdentifier = makeObject<SetIdentifier>(
            aggregator->getDatabaseName(), aggregator->getSetName());

      } else if (sourceComputation->getComputationType() == "SelectionComp") {
        Handle<SelectionComp<Object, Object>> selector =
            unsafeCast<SelectionComp<Object, Object>, Computation>(
                sourceComputation);
        curInputSetIdentifier = makeObject<SetIdentifier>(
            selector->getDatabaseName(), selector->getSetName());
      } else if (sourceComputation->getComputationType() ==
                 "MultiSelectionComp") {
        Handle<MultiSelectionComp<Object, Object>> selector =
            unsafeCast<MultiSelectionComp<Object, Object>, Computation>(
                sourceComputation);
        curInputSetIdentifier = makeObject<SetIdentifier>(
            selector->getDatabaseName(), selector->getSetName());
      } else if (sourceComputation->getComputationType() == 
                 "PartitionComp") {
        Handle<PartitionComp<Object, Object>> partitioner = 
            unsafeCast<PartitionComp<Object, Object>, Computation>(sourceComputation);
        curInputSetIdentifier = makeObject<SetIdentifier>(partitioner->getDatabaseName(), partitioner->getSetName());
      } else {
        std::cout << "Source Computation Type: "
                  << sourceComputation->getComputationType()
                  << " are not supported as source node right now" << std::endl;
        this->logger->fatal("Source Computation Type: " +
                            sourceComputation->getComputationType() +
                            " are not supported as source node right now");
        std::cout << "Master exit...Please restart cluster" << std::endl;
        exit(1);
      }
      std::string mySourceSetName = curInputSetIdentifier->getDatabase() + ":" +
                                    curInputSetIdentifier->getSetName();
      int j = 0;
      for (; j < curSourceSetNames.size(); j++) {
        if (curSourceSetNames[j].compare(mySourceSetName) == 0) {
          break;
        }
      }
      if (j == curSourceSetNames.size()) {
        curSourceSetNames.push_back(mySourceSetName);
        curSourceSets[mySourceSetName] = curInputSetIdentifier;
        curProcessedConsumers[mySourceSetName] = 0;
        curSourceNodes[mySourceSetName].push_back(curSource);
      } else {
        curSourceNodes[mySourceSetName].push_back(curSource);
      }
    }
  }
}

TCAPAnalyzer::~TCAPAnalyzer() {
  this->sources.clear();
  this->logicalPlan = nullptr;
  if (this->computePlan != nullptr) {
      this->computePlan->nullifyPlanPointer();
      this->computePlan = nullptr;
  }
  this->hashSetsToProbe = nullptr;
}

bool TCAPAnalyzer::analyze(
    std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
    std::vector<Handle<SetIdentifier>> &interGlobalSets) {
  int jobStageId = 0;
  for (int i = 0; i < this->sources.size(); i++) {
    AtomicComputationPtr curSource = sources[i];
    bool ret =
        analyze(physicalPlanToOutput, interGlobalSets, curSource, jobStageId);
    if (ret == false) {
      return false;
    }
  }
  return true;
}

// to analyze the subgraph rooted at a source node and only returns a set of job
// stages
// corresponding with the subgraph
bool TCAPAnalyzer::getNextStages(
    std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
    std::vector<Handle<SetIdentifier>> &interGlobalSets,
    AtomicComputationPtr curSource, Handle<SetIdentifier> curInputSetIdentifier,
    int &jobStageId) {
  std::string sourceSpecifier = curSource->getComputationName();
  Handle<Computation> sourceComputation =
      this->logicalPlan->getNode(sourceSpecifier).getComputationHandle();
  std::string outputName = curSource->getOutputName();
  std::vector<AtomicComputationPtr> consumers =
      this->computationGraph.getConsumingAtomicComputations(outputName);
  int numConsumersForCurNode = consumers.size();
  for (int i = 0; i < numConsumersForCurNode; i++) {
    AtomicComputationPtr curNode = consumers[i];
    std::vector<std::string> tupleSetNames;
    tupleSetNames.push_back(outputName);
    bool ret = analyze(physicalPlanToOutput, interGlobalSets, tupleSetNames,
                       curSource, sourceComputation, curInputSetIdentifier,
                       curNode, jobStageId, curSource, false, UnknownJoin, defaultAllocator, "", sourceSpecifier);
    if (ret == false) {
      return false;
    }
  }
  return true;
}

// to analyze the subgraph rooted at a source node and only returns a set of job
// stages
// corresponding with the subgraph
bool TCAPAnalyzer::getNextStagesOptimized(
    std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
    std::vector<Handle<SetIdentifier>> &interGlobalSets,
    AtomicComputationPtr curSource, Handle<SetIdentifier> curInputSetIdentifier,
    unsigned int curConsumerIndex, int &jobStageId) {
  std::string sourceSpecifier = curSource->getComputationName();
  Handle<Computation> sourceComputation =
      this->logicalPlan->getNode(sourceSpecifier).getComputationHandle();
  std::string outputName = curSource->getOutputName();
  std::vector<AtomicComputationPtr> consumers =
      this->computationGraph.getConsumingAtomicComputations(outputName);
  AtomicComputationPtr curNode = nullptr;
  bool ret = false;
  if (curConsumerIndex < consumers.size()) {
    curNode = consumers[curConsumerIndex];
    std::vector<std::string> tupleSetNames;
    tupleSetNames.push_back(outputName);
    ret = analyze(physicalPlanToOutput, interGlobalSets, tupleSetNames,
                  curSource, sourceComputation, curInputSetIdentifier, curNode,
                  jobStageId, curSource, false, UnknownJoin, defaultAllocator, "", sourceSpecifier);
    if (ret == false) {
      return true;
    }
  }
  return ret;
}

// to get number of consumers for a certain source
unsigned int TCAPAnalyzer::getNumConsumers(std::string name) {
  if (this->getSourceSetIdentifier(name) == nullptr) {
    return 0;
  }
  AtomicComputationPtr sourceAtomicComp = this->getSourceComputation(name);
  std::string outputName = sourceAtomicComp->getOutputName();
  std::vector<AtomicComputationPtr> consumers =
      this->computationGraph.getConsumingAtomicComputations(outputName);
  unsigned int curConsumerIndex = this->getNextConsumerIndex(name);
  return (consumers.size() - curConsumerIndex);
}

// a source computation for a pipeline can be ScanSet, Selection, Partition
// ClusterAggregation, and
// ClusterJoin.
bool TCAPAnalyzer::analyze(
    std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
    std::vector<Handle<SetIdentifier>> &interGlobalSets,
    AtomicComputationPtr curSource, int &jobStageId) {
  // first get source set identifier
  std::string sourceSpecifier = curSource->getComputationName();
  Handle<SetIdentifier> curInputSetIdentifier = nullptr;
  Handle<Computation> sourceComputation =
      this->logicalPlan->getNode(sourceSpecifier).getComputationHandle();
  if ((sourceComputation->getComputationType() == "ScanUserSet") ||
      (sourceComputation->getComputationType() == "ScanSet")) {
    Handle<ScanUserSet<Object>> scanner =
        unsafeCast<ScanUserSet<Object>, Computation>(sourceComputation);
    curInputSetIdentifier = makeObject<SetIdentifier>(
        scanner->getDatabaseName(), scanner->getSetName());
  } else if (sourceComputation->getComputationType() ==
             "ClusterAggregationComp") {
    Handle<AbstractAggregateComp> aggregator =
        unsafeCast<AbstractAggregateComp, Computation>(sourceComputation);
    curInputSetIdentifier = makeObject<SetIdentifier>(
        aggregator->getDatabaseName(), aggregator->getSetName());

  } else if (sourceComputation->getComputationType() == "SelectionComp") {
    Handle<SelectionComp<Object, Object>> selector =
        unsafeCast<SelectionComp<Object, Object>, Computation>(
            sourceComputation);
    curInputSetIdentifier = makeObject<SetIdentifier>(
        selector->getDatabaseName(), selector->getSetName());

  } else if (sourceComputation->getComputationType() == "MultiSelectionComp") {
    Handle<MultiSelectionComp<Object, Object>> selector =
        unsafeCast<MultiSelectionComp<Object, Object>, Computation>(
            sourceComputation);
    curInputSetIdentifier = makeObject<SetIdentifier>(
        selector->getDatabaseName(), selector->getSetName());
  } else if (sourceComputation->getComputationType() ==
                 "PartitionComp") {
    Handle<PartitionComp<Object, Object>> partitioner =
        unsafeCast<PartitionComp<Object, Object>, Computation>(sourceComputation);
    curInputSetIdentifier = makeObject<SetIdentifier>(partitioner->getDatabaseName(), partitioner->getSetName());
      
  } else {
    std::cout << "Source Computation Type: "
              << sourceComputation->getComputationType()
              << " are not supported as source node right now" << std::endl;
    this->logger->fatal("Source Computation Type: " +
                        sourceComputation->getComputationType() +
                        " are not supported as source node right now");
    exit(1);
  }

  std::string outputName = curSource->getOutputName();
  std::vector<AtomicComputationPtr> consumers =
      this->computationGraph.getConsumingAtomicComputations(outputName);
  int numConsumersForCurNode = consumers.size();
  for (int i = 0; i < numConsumersForCurNode; i++) {
    AtomicComputationPtr curNode = consumers[i];
    std::vector<std::string> tupleSetNames;
    tupleSetNames.push_back(outputName);
    bool ret = analyze(physicalPlanToOutput, interGlobalSets, tupleSetNames,
                       curSource, sourceComputation, curInputSetIdentifier,
                       curNode, jobStageId, curSource, false, UnknownJoin, defaultAllocator, "", sourceSpecifier);
    if (ret == false) {
      std::cout << "We met a penalized source set and simply return."
                << std::endl;
      return true;
    }
  }
  return true;
}

Handle<TupleSetJobStage> TCAPAnalyzer::createTupleSetJobStage(
    int &jobStageId, std::string sourceTupleSetName,
    std::string targetTupleSetName, std::string targetComputationName,
    std::vector<std::string> buildTheseTupleSets, std::string outputTypeName,
    Handle<SetIdentifier> sourceContext, Handle<SetIdentifier> combinerContext,
    Handle<SetIdentifier> sinkContext, bool isBroadcasting,
    bool isRepartitioning, bool needsRemoveInputSet, bool isProbing, JoinType joinType,
    AllocatorPolicy myPolicy, bool isRepartitionJoin, bool isCollectAsMap,
    int numNodesToCollect, bool hasLocalJoinSink, bool hasLocalJoinProbe,
    std::string partitionComputationName, std::string partitionLambdaName) {
  Handle<TupleSetJobStage> jobStage = makeObject<TupleSetJobStage>(jobStageId);
  jobStageId++;
  jobStage->setComputePlan(this->computePlan, sourceTupleSetName,
                           targetTupleSetName, targetComputationName);
  jobStage->setTupleSetsToBuildPipeline(buildTheseTupleSets);
  jobStage->setSourceContext(sourceContext);
  jobStage->setSinkContext(sinkContext);
  jobStage->setOutputTypeName(outputTypeName);
  jobStage->setAllocatorPolicy(myPolicy);
  jobStage->setRepartitionJoin(isRepartitionJoin);
  jobStage->setLocalJoinSinkOrNot(hasLocalJoinSink);
  jobStage->setLocalJoinProbeOrNot(hasLocalJoinProbe);
  jobStage->setPartitionComputationSpecifier(partitionComputationName);
  jobStage->setPartitionLambdaName(partitionLambdaName);
  if ((hashSetsToProbe != nullptr) && (outputForJoinSets.size() > 0) &&
      (isProbing == true)) {
    jobStage->setProbing(true);
    Handle<Map<String, String>> hashSetToProbeForMe =
        makeObject<Map<String, String>>();
    for (int i = 0; i < outputForJoinSets.size(); i++) {
      (*hashSetToProbeForMe)[outputForJoinSets[i]] =
          (*hashSetsToProbe)[outputForJoinSets[i]];
    }
    jobStage->setHashSetsToProbe(hashSetToProbeForMe);
    outputForJoinSets.clear();
  }

  if (isProbing == true) {
    jobStage->setJoinType(joinType);
  }
  if (combinerContext != nullptr) {
    jobStage->setCombinerContext(combinerContext);
    jobStage->setCombining(true);
    jobStage->setNeedsRemoveCombinerSet(true);
  }
  jobStage->setNeedsRemoveInputSet(needsRemoveInputSet);
  if (sourceContext->isAggregationResult() == true) {
    jobStage->setInputAggHashOut(true);
    jobStage->setNeedsRemoveInputSet(
        true); // aggregation output should not be kept across
               // stages; if an aggregation has more than one
               // consumers, we need materialize aggregation
               // results.
  }
  jobStage->setBroadcasting(isBroadcasting);
  jobStage->setRepartition(isRepartitioning);
  jobStage->setJobId(this->jobId);
  jobStage->setCollectAsMap(isCollectAsMap);
  jobStage->setNumNodesToCollect(numNodesToCollect);
  PDB_COUT << "TCAPAnalyzer generates tupleSetJobStage:" << std::endl;
  return jobStage;
}

Handle<AggregationJobStage> TCAPAnalyzer::createAggregationJobStage(
    int &jobStageId, Handle<AbstractAggregateComp> aggComp,
    Handle<SetIdentifier> sourceContext, Handle<SetIdentifier> sinkContext,
    std::string outputTypeName, bool materializeResultOrNot) {
  Handle<AggregationJobStage> aggStage = makeObject<AggregationJobStage>(
      jobStageId, materializeResultOrNot, aggComp);
  jobStageId++;
  aggStage->setSourceContext(sourceContext);
  aggStage->setSinkContext(sinkContext);
  aggStage->setOutputTypeName(aggComp->getOutputType());
  aggStage->setNeedsRemoveInputSet(false);
  aggStage->setJobId(this->jobId);
  return aggStage;
}

Handle<BroadcastJoinBuildHTJobStage>
TCAPAnalyzer::createBroadcastJoinBuildHTJobStage(
    int &jobStageId, std::string sourceTupleSetName,
    std::string targetTupleSetName, std::string targetComputationName,
    Handle<SetIdentifier> sourceContext, std::string hashSetName,
    bool needsRemoveInputSet) {
  Handle<BroadcastJoinBuildHTJobStage> broadcastJoinStage =
      makeObject<BroadcastJoinBuildHTJobStage>(this->jobId, jobStageId,
                                               hashSetName);
  jobStageId++;
  broadcastJoinStage->setComputePlan(this->computePlan, sourceTupleSetName,
                                     targetTupleSetName, targetComputationName);
  broadcastJoinStage->setSourceContext(sourceContext);
  broadcastJoinStage->setNeedsRemoveInputSet(needsRemoveInputSet);
  return broadcastJoinStage;
}

Handle<HashPartitionedJoinBuildHTJobStage>
TCAPAnalyzer::createHashPartitionedJoinBuildHTJobStage(
    int &jobStageId, std::string sourceTupleSetName,
    std::string targetTupleSetName, std::string targetComputationName,
    Handle<SetIdentifier> sourceContext, std::string hashSetName,
    bool needsRemoveInputSet) {
  Handle<HashPartitionedJoinBuildHTJobStage> hashPartitionedJobStage =
      makeObject<HashPartitionedJoinBuildHTJobStage>(this->jobId, jobStageId,
                                                     hashSetName);
  jobStageId++;
  hashPartitionedJobStage->setComputePlan(this->computePlan, sourceTupleSetName,
                                          targetTupleSetName,
                                          targetComputationName);
  hashPartitionedJobStage->setSourceContext(sourceContext);
  hashPartitionedJobStage->setNeedsRemoveInputSet(needsRemoveInputSet);
  return hashPartitionedJobStage;
}

// to remove source
bool TCAPAnalyzer::removeSource(std::string oldSetName) {
  bool ret;
  if (curSourceNodes.count(oldSetName) > 0) {
    if (curSourceNodes[oldSetName].size() > 0) {
      curSourceNodes[oldSetName].erase(curSourceNodes[oldSetName].begin());
      curProcessedConsumers[oldSetName] = 0;
    }
    if (curSourceNodes[oldSetName].size() > 0) {
      return true;
    } else {
      curSourceNodes.erase(oldSetName);
    }
  }
  for (std::vector<std::string>::iterator iter = curSourceSetNames.begin();
       iter != curSourceSetNames.end(); iter++) {
    PDB_COUT << "curSetName = " << *iter << std::endl;
    std::string curStr = *iter;
    if (curStr == oldSetName) {
      iter = curSourceSetNames.erase(iter);
      break;
    }
  }

  for (std::vector<std::string>::iterator iter = penalizedSourceSets.begin();
       iter != penalizedSourceSets.end(); iter++) {
    PDB_COUT << "curSetName = " << *iter << std::endl;
    std::string curStr = *iter;
    if (curStr == oldSetName) {
      iter = penalizedSourceSets.erase(iter);
      
      break;
    }
  }

  if (curSourceSets.count(oldSetName) > 0) {
    curSourceSets.erase(oldSetName);
  } else {
    ret = false;
  }

  if (curProcessedConsumers.count(oldSetName) > 0) {
    curProcessedConsumers.erase(oldSetName);
  } else {
    ret = false;
  }

  return ret;
}

bool TCAPAnalyzer::updateSourceSets(Handle<SetIdentifier> oldSet,
                                    Handle<SetIdentifier> newSet,
                                    AtomicComputationPtr newAtomicComp) {
  bool ret = true;
  if ((newSet != nullptr) && (newAtomicComp != nullptr)) {
    // get new set name
    std::string newSetName = newSet->getDatabase() + ":" + newSet->getSetName();
    // add new set
    curSourceSetNames.push_back(newSetName);
    curSourceSets[newSetName] = newSet;
    curSourceNodes[newSetName].push_back(newAtomicComp);
    if (curProcessedConsumers.count(newSetName) == 0) {
      curProcessedConsumers[newSetName] = 0;
    }
  }
  // get old set name
  std::string oldSetName = oldSet->getDatabase() + ":" + oldSet->getSetName();

  // check current string
  for (std::string &myStr : curSourceSetNames) {
    PDB_COUT << myStr << std::endl;
  }

  // remove the old stuff
  AtomicComputationPtr oldNode;
  if (curSourceNodes.count(oldSetName) > 0) {
    oldNode = curSourceNodes[oldSetName][0];
  } else {
    return false;
  }

  std::string sourceSpecifier = oldNode->getComputationName();
  Handle<Computation> sourceComputation =
      this->logicalPlan->getNode(sourceSpecifier).getComputationHandle();
  std::string outputName = oldNode->getOutputName();
  std::vector<AtomicComputationPtr> consumers =
      this->computationGraph.getConsumingAtomicComputations(outputName);
  unsigned int numConsumers = consumers.size();
  unsigned int numProcessedConsumers = curProcessedConsumers[oldSetName];
  if (numConsumers == numProcessedConsumers) {
    removeSource(oldSetName);
  }
  return ret;
}

//PartitionComp cannot have any consummers.
bool TCAPAnalyzer::analyze(
    std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
    std::vector<Handle<SetIdentifier>> &interGlobalSets,
    std::vector<std::string> &buildTheseTupleSets,
    AtomicComputationPtr curSource, Handle<Computation> sourceComputation,
    Handle<SetIdentifier> curInputSetIdentifier, AtomicComputationPtr curNode,
    int &jobStageId, AtomicComputationPtr prevNode, bool isProbing, JoinType joinType,
    AllocatorPolicy myPolicy, std::string joinSource, std::string prevComputationName, bool hasLocalJoinProbe, std::string partitionComputationSpecifier, std::string partitionLambdaName) {
  // to get consumers
  std::string outputName = curNode->getOutputName();
  std::vector<AtomicComputationPtr> consumers =
      this->computationGraph.getConsumingAtomicComputations(outputName);
  int numConsumersForCurNode = consumers.size();
  
  // to get my type
  std::string mySpecifier = curNode->getComputationName();
  Handle<Computation> myComputation =
      this->logicalPlan->getNode(mySpecifier).getComputationHandle();

  if (myComputation->getAllocatorPolicy() !=
      AllocatorPolicy::defaultAllocator) {
    myPolicy = myComputation->getAllocatorPolicy();
  }

  if (numConsumersForCurNode == 0) {
    // to get my output set
    std::string dbName = myComputation->getDatabaseName();
    std::string setName = myComputation->getSetName();
    Handle<SetIdentifier> sink = makeObject<SetIdentifier>(dbName, setName);
    if (myComputation->getComputationType() == "ClusterAggregationComp") {
      // to create the producing job stage for aggregation
      Handle<SetIdentifier> aggregator = makeObject<SetIdentifier>(
          this->jobId, outputName + "_aggregationData");
      size_t desiredSize = this->exactSizeOfCurSource/conf->getShufflePageSize()/this->numNodesInCluster;
      if (desiredSize == 0) {
          desiredSize = 1;
      }
      aggregator->setDesiredSize(desiredSize);
      aggregator->setPageSize(conf->getShufflePageSize());
      Handle<SetIdentifier> combiner = nullptr;
      if (myComputation->isUsingCombiner() == true) {
        combiner = makeObject<SetIdentifier>(this->jobId,
                                             outputName + "_combinerData");
      }
      std::string sourceTupleSetName = curSource->getOutputName();
      if (joinSource != "") {
        sourceTupleSetName = joinSource;
        joinSource = "";
      }
      Handle<AbstractAggregateComp> agg =
          unsafeCast<AbstractAggregateComp, Computation>(myComputation);
      Handle<TupleSetJobStage> jobStage = createTupleSetJobStage(
          jobStageId, sourceTupleSetName, curNode->getInputName(), mySpecifier,
          buildTheseTupleSets, "IntermediateData", curInputSetIdentifier,
          combiner, aggregator, false, true, false, isProbing, joinType, myPolicy, false,
          agg->isCollectAsMap(), agg->getNumNodesToCollect(), false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
      // to push back the job stage
      physicalPlanToOutput.push_back(jobStage);
      // to create the consuming job stage for aggregation
      Handle<AggregationJobStage> aggStage = createAggregationJobStage(
          jobStageId, agg, aggregator, sink, agg->getOutputType(), true);
      // to push back the aggregation stage;
      physicalPlanToOutput.push_back(aggStage);
      // to push back the aggregator set
      interGlobalSets.push_back(aggregator);
      if (this->dynamicPlanningOrNot == true) {
        this->updateSourceSets(curInputSetIdentifier, nullptr, nullptr);
      }
      return true;

    } else if ((myComputation->getComputationType() == "WriteUserSet") ||
               (myComputation->getComputationType() == "SelectionComp") ||
               (myComputation->getComputationType() == "MultiSelectionComp")) {
      std::string sourceTupleSetName = curSource->getOutputName();
      if (joinSource != "") {
        sourceTupleSetName = joinSource;
        joinSource = "";
      }
      Handle<TupleSetJobStage> jobStage = createTupleSetJobStage(
          jobStageId, sourceTupleSetName, curNode->getInputName(), mySpecifier,
          buildTheseTupleSets, myComputation->getOutputType(),
          curInputSetIdentifier, nullptr, sink, false, false, false, isProbing, joinType,
          myPolicy, false, 0, 0, false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
      physicalPlanToOutput.push_back(jobStage);
      if (this->dynamicPlanningOrNot == true) {
        this->updateSourceSets(curInputSetIdentifier, nullptr, nullptr);
      }
      return true;
    } else if (myComputation->getComputationType() == "PartitionComp") {
      //TODO: PartitionComp should configure the dataset to have partitioner lambdas, so that the partition can be utilized for local join.
      std::string sourceTupleSetName = curSource->getOutputName();
      if (joinSource != "") {
        sourceTupleSetName = joinSource;
        joinSource = "";
      }
      Handle<TupleSetJobStage> jobStage = createTupleSetJobStage(
          jobStageId, sourceTupleSetName, curNode->getInputName(), mySpecifier,
          buildTheseTupleSets, myComputation->getOutputType(),
          curInputSetIdentifier, nullptr, sink, false, true, false, isProbing, joinType,
          myPolicy, false, 0, 0, false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
      physicalPlanToOutput.push_back(jobStage);
      if (this->dynamicPlanningOrNot == true) {
        this->updateSourceSets(curInputSetIdentifier, nullptr, nullptr);
      }
      return true;

    } else {
      std::cout << "Sink Computation Type: "
                << myComputation->getComputationType()
                << " are not supported as sink node right now" << std::endl;
      this->logger->fatal("Source Computation Type: " +
                          myComputation->getComputationType() +
                          " are not supported as sink node right now");
      exit(1);
    }

  } else if (numConsumersForCurNode == 1) {
    if (myComputation->getComputationType() == "PartitionComp" && consumers[0]->getAtomicComputationType() != "Partition") {
      std::cout << "PartitionComp "
                << "cannot have any consumer node right now" << std::endl;
      this->logger->fatal("PartitionComp cannot have any consumer node right now");
      exit(1);
    }
    AtomicComputationPtr nextNode = consumers[0];
    //compare nextNode's computation and myComputation
    std::string nextSpecifier = nextNode->getComputationName();
    std::string prevSpecifier = prevComputationName;
    if (nextSpecifier != mySpecifier) {
         prevSpecifier = mySpecifier;
    }
    // check my type
    // if I am aggregation or join, I am a pipeline breaker
    // we currently do not support join
    if (curNode->getAtomicComputationType() == "Aggregate") {
      Handle<SetIdentifier> aggregator = makeObject<SetIdentifier>(
          this->jobId, outputName + "_aggregationData");
      size_t desiredSize = this->exactSizeOfCurSource/conf->getShufflePageSize()/this->numNodesInCluster;
      if (desiredSize == 0) {
          desiredSize = 1;
      }
      aggregator->setDesiredSize(desiredSize);
      aggregator->setPageSize(conf->getShufflePageSize());
      Handle<SetIdentifier> combiner = nullptr;
      if (myComputation->isUsingCombiner() == true) {
        combiner = makeObject<SetIdentifier>(this->jobId,
                                             outputName + "_combinerData");
      }
      std::string sourceTupleSetName = curSource->getOutputName();
      if (joinSource != "") {
        sourceTupleSetName = joinSource;
        joinSource = "";
      }
      Handle<AbstractAggregateComp> agg =
          unsafeCast<AbstractAggregateComp, Computation>(myComputation);
      Handle<TupleSetJobStage> jobStage = createTupleSetJobStage(
          jobStageId, sourceTupleSetName, curNode->getInputName(), mySpecifier,
          buildTheseTupleSets, "IntermediateData", curInputSetIdentifier,
          combiner, aggregator, false, true, false, isProbing, joinType, myPolicy, false,
          agg->isCollectAsMap(), agg->getNumNodesToCollect(), false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
      physicalPlanToOutput.push_back(jobStage);
      // to create the consuming job stage for aggregation
      Handle<AggregationJobStage> aggStage;
      Handle<SetIdentifier> sink;
      if (myComputation->needsMaterializeOutput() == true) {
        // to get my output set
        std::string dbName = myComputation->getDatabaseName();
        std::string setName = myComputation->getSetName();
        sink = makeObject<SetIdentifier>(dbName, setName);
        aggStage = createAggregationJobStage(jobStageId, agg, aggregator, sink,
                                             agg->getOutputType(), true);
      } else {
        sink = makeObject<SetIdentifier>(this->jobId,
                                         outputName + "_aggregationResult",
                                         PartitionedHashSetType, true);
        aggStage = createAggregationJobStage(jobStageId, agg, aggregator, sink,
                                             agg->getOutputType(), false);
      }
      // to push back the aggregation stage;
      physicalPlanToOutput.push_back(aggStage);
      // to push back the aggregator set
      interGlobalSets.push_back(aggregator);
      if (this->dynamicPlanningOrNot == true) {
        this->updateSourceSets(curInputSetIdentifier, sink, curNode);
        return true;
      } else {
        buildTheseTupleSets.clear();
        buildTheseTupleSets.push_back(curNode->getOutputName());
        // Now I am the source!
        
        return analyze(physicalPlanToOutput, interGlobalSets,
                       buildTheseTupleSets, curNode, myComputation, sink,
                       nextNode, jobStageId, curNode, false, joinType, defaultAllocator, "", prevSpecifier, false);
      }
    } else if (curNode->getAtomicComputationType() == "JoinSets") {
      std::shared_ptr<ApplyJoin> joinNode =
          dynamic_pointer_cast<ApplyJoin>(curNode);
      std::string targetTupleSetName;
      if (prevNode == nullptr) {
        targetTupleSetName =
            curNode->getInputName(); // join has two input names
      } else {
        targetTupleSetName = prevNode->getOutputName();
      }
      Handle<SetIdentifier> sink = nullptr;
      std::string hashSetName = "";
      if (joinNode->isTraversed() == false) {
        if ((isProbing == true) &&
            (this->costOfCurSource <= JOIN_COST_THRESHOLD)) {
          // this is too bad that we we've already probed join tables and join
          // results
          // could be huge.
          // so we do nothing just clear the pipeline stages and return;
          buildTheseTupleSets.clear();
          // update current source set cost;
          size_t j = 0;
          for (j = 0; j < penalizedSourceSets.size(); j++) {
              if (this->curSourceSetName == penalizedSourceSets[j]) {
                  break;
              }
          }
          if ((penalizedSourceSets.size() == 0) ||
              (j == penalizedSourceSets.size())) {
            std::cout
                << "WARNING: met a join sink with probing, to return and put "
                << this->curSourceSetName << " to penalized list" << std::endl;
            penalizedSourceSets.push_back(this->curSourceSetName);
          }
          return false;
        }
        if (this->costOfCurSource > JOIN_COST_THRESHOLD) {
          // data is larger than the cost threshold, so we do hash partition
          // join.
          joinNode->setPartitioningLHS(true);
          Handle<JoinComp<Object, Object, Object>> join =
              unsafeCast<JoinComp<Object, Object, Object>, Computation>(
                  myComputation);


          // I am a pipeline breaker.
          // We first need to create a TupleSetJobStage with a repartition sink

          sink = makeObject<SetIdentifier>(this->jobId,
                                           outputName + "_repartitionData");
          sink->setPageSize(conf->getBroadcastPageSize());
          size_t desiredSize = (size_t)this->exactSizeOfCurSource/(size_t)conf->getBroadcastPageSize()/(size_t)this->numNodesInCluster;
          if (desiredSize == 0) {
              desiredSize = 1;
          }
          sink->setDesiredSize(desiredSize);
          // isBroadcasting = false
          // isRepartitioning = true
          // collect probing information
          // isCombining = false
          std::string sourceTupleSetName = curSource->getOutputName();
          if (joinSource != "") {
             sourceTupleSetName = joinSource;
             joinSource = "";
          }

          std::string partitionComputationName = "";
          std::string partitionLambdaName = "";
          bool matchOrNot = matchSourceWithQuery(this->jobId, 
                                                 curSource, 
                                                 joinNode, 
                                                 logicalPlan, 
                                                 curInputSetIdentifier,
                                                 partitionComputationName,
                                                 partitionLambdaName);

          int indexInInputs = -1;
          indexInInputs = join->getIndexInInputsByComputationName(prevComputationName);
          
          if ((indexInInputs < 0) || (indexInInputs > join->getNumInputs())) {
              indexInInputs = join->getIndexInInputsByTypeName(curInputSetIdentifier->getDataType());
          }
          if ((indexInInputs < 0) || (indexInInputs > join->getNumInputs())) {
              indexInInputs = 0;
          }
          curInputSetIdentifier->setIndexInInputs(indexInInputs);

          //we need to probe the other side
          std::vector<std::pair<std::string, std::string>> scannerSources;
          std::pair<std::string, std::string> curScannerSource;
          join->getSources(scannerSources);
          Handle<SetIdentifier> rhsSourceIdentifier = nullptr;
          for (int k = 0; k < scannerSources.size(); k++) {
              curScannerSource = scannerSources[k];
              if ((curScannerSource.first == curInputSetIdentifier->getDatabase())
                  &&(curScannerSource.second == curInputSetIdentifier->getSetName())) {
                  continue;
              } else {
                  rhsSourceIdentifier = makeObject<SetIdentifier>(curScannerSource.first, curScannerSource.second);
                  break;
              }
          }
          std::string otherSourceSetName = curScannerSource.first + ":" + curScannerSource.second;
          std::vector<AtomicComputationPtr> rhsSources = curSourceNodes[otherSourceSetName];
          std::string rhsPartitionComputationName = "";
          std::string rhsPartitionLambdaName = "";
          bool otherMatchOrNot = false;
          
          if ((matchOrNot == true)&&(rhsSources.size()>0)) {
              otherMatchOrNot = matchSourceWithQuery(this->jobId,
                                                 rhsSources[0],
                                                 joinNode,
                                                 logicalPlan,
                                                 rhsSourceIdentifier,
                                                 rhsPartitionComputationName,
                                                 rhsPartitionLambdaName);

          }
          if ((matchOrNot == false)||(otherMatchOrNot == false)) {
              if (join->getJoinType() != CrossProduct)
		  join->setJoinType(HashPartitionedJoin);
              hashSetName = sink->getDatabase() + ":" + sink->getSetName();
              Handle<TupleSetJobStage> joinPrepStage = createTupleSetJobStage(
                 jobStageId, sourceTupleSetName, targetTupleSetName, mySpecifier,
                 buildTheseTupleSets, "IntermediateData", curInputSetIdentifier,
                 nullptr, sink, false, true, false, isProbing, join->getJoinType(), myPolicy, true, false, 0, 
                 false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
	      if (join->getJoinType()==CrossProduct) {
	      
		  joinPrepStage->setJoinType("CrossProduct");
	      
	      } else if (join->getJoinType()==HashPartitionedJoin) {

		  joinPrepStage->setJoinType("HashPartitionedJoin");

              } else if (join->getJoinType()==LocalJoin) {

		  joinPrepStage->setJoinType("LocalJoin");
	      
	      } else if (join->getJoinType()==BroadcastJoin) {
	      
                  joinPrepStage->setJoinType("BroadcastJoin");

	      } else {
	          
                  joinPrepStage->setJoinType("UnknownJoinType");
	      }
              physicalPlanToOutput.push_back(joinPrepStage);
              interGlobalSets.push_back(sink);

              // We then create a HashPartitionedJoinBuildHTStage
              Handle<HashPartitionedJoinBuildHTJobStage> joinPartitionStage =
                  createHashPartitionedJoinBuildHTJobStage(
                     jobStageId, sourceTupleSetName, targetTupleSetName,
                     mySpecifier, sink, hashSetName, false);
              physicalPlanToOutput.push_back(joinPartitionStage);
          } else {
              join->setJoinType(LocalJoin);
              if ((sourceComputation->getComputationType() == "ScanUserSet") ||
                 (sourceComputation->getComputationType() == "ScanSet")) {
                     Handle<ScanUserSet<Object>> scanner =
                        unsafeCast<ScanUserSet<Object>, Computation>(sourceComputation);
                     scanner->setFollowedByLocalJoin(true);
              }
              //we simply build hash table for local join
              hashSetName = sink->getDatabase() + ":" + sink->getSetName();
              Handle<TupleSetJobStage> joinPrepStage = createTupleSetJobStage(
                 jobStageId, sourceTupleSetName, targetTupleSetName, mySpecifier,
                 buildTheseTupleSets, "IntermediateData", curInputSetIdentifier,
                 nullptr, sink, false, true, false, isProbing, join->getJoinType(), myPolicy, false, false, 0,
                 true, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
              joinPrepStage->setPartitionComputationSpecifier(partitionComputationName);
              joinPrepStage->setPartitionLambdaName(partitionLambdaName);
              physicalPlanToOutput.push_back(joinPrepStage);
          }

        } else {
          // we are doing broadcast join

          // if the other input has not been processed, I am a pipeline breaker.
          // We first need to create a TupleSetJobStage with a broadcasting sink

          sink = makeObject<SetIdentifier>(this->jobId,
                                           outputName + "_broadcastData");
          sink->setPageSize(conf->getBroadcastPageSize());
          size_t desiredSize = this->exactSizeOfCurSource/conf->getBroadcastPageSize()/this->numNodesInCluster;
          if (desiredSize == 0) {
              desiredSize = 1;
          }
          sink->setDesiredSize(desiredSize);
          // isBroadcasting = true
          // isRepartitioning = false
          // collect probing information
          // isCombining = false
          std::string sourceTupleSetName = curSource->getOutputName();
          if (joinSource != "") {
            sourceTupleSetName = joinSource;
            joinSource = "";
          }
          Handle<JoinComp<Object, Object, Object>> join =
              unsafeCast<JoinComp<Object, Object, Object>, Computation>(
                  myComputation);

          int indexInInputs = -1;
          indexInInputs = join->getIndexInInputsByComputationName(prevComputationName);
          
          if ((indexInInputs < 0) || (indexInInputs > join->getNumInputs())) {
              indexInInputs = join->getIndexInInputsByTypeName(curInputSetIdentifier->getDataType());
          }
          if ((indexInInputs < 0) || (indexInInputs > join->getNumInputs())) {
              indexInInputs = 0;
          }
          curInputSetIdentifier->setIndexInInputs(indexInInputs);
          Handle<TupleSetJobStage> joinPrepStage = createTupleSetJobStage(
              jobStageId, sourceTupleSetName, targetTupleSetName, mySpecifier,
              buildTheseTupleSets, "IntermediateData", curInputSetIdentifier,
              nullptr, sink, true, false, false, isProbing, join->getJoinType(), myPolicy, false, false, 0,
              false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
          physicalPlanToOutput.push_back(joinPrepStage);
          interGlobalSets.push_back(sink);

          // We then create a BroadcastJoinBuildHTStage
          hashSetName = sink->getDatabase() + ":" + sink->getSetName();
          Handle<BroadcastJoinBuildHTJobStage> joinBroadcastStage =
              createBroadcastJoinBuildHTJobStage(
                  jobStageId, sourceTupleSetName, targetTupleSetName,
                  mySpecifier, sink, hashSetName, false);
          physicalPlanToOutput.push_back(joinBroadcastStage);
        }
        // set the probe information
        if (hashSetsToProbe == nullptr) {
          hashSetsToProbe = makeObject<Map<String, String>>();
        }
        (*hashSetsToProbe)[outputName] = hashSetName;
        // We should not go further, we set it to traversed and leave it to
        // other join
        // inputs, and simply return
        joinNode->setTraversed(true);
        if (this->dynamicPlanningOrNot == true) {
          updateSourceSets(curInputSetIdentifier, nullptr, nullptr);
        }
        return true;
	
      } else {
        if (joinNode->isPartitioningLHS() == true) {
          // we probe the partitioned hash table
          // we first create a pipeline breaker to partition RHS
          sink = makeObject<SetIdentifier>(this->jobId,
                                           outputName + "_probingData");
          sink->setPageSize(conf->getBroadcastPageSize());
          size_t desiredSize = this->exactSizeOfCurSource/conf->getBroadcastPageSize()/this->numNodesInCluster;
          if (desiredSize == 0) {
              desiredSize = 1;
          }
          sink->setDesiredSize(desiredSize);
          // isBroadcasting = false
          // isRepartitioning = true
          // collect probing information
          // isCombining = false
          std::string sourceTupleSetName = curSource->getOutputName();
          if (joinSource != "") {
            sourceTupleSetName = joinSource;
            joinSource = "";
          }

          std::string partitionComputationName = "";
          std::string partitionLambdaName = "";
          bool matchOrNot = matchSourceWithQuery(this->jobId,
                                                 curSource,
                                                 joinNode,
                                                 logicalPlan,
                                                 curInputSetIdentifier,
                                                 partitionComputationName,
                                                 partitionLambdaName);

           


          Handle<JoinComp<Object, Object, Object>> join =
              unsafeCast<JoinComp<Object, Object, Object>, Computation>(
                  myComputation);


	  
          int indexInInputs = -1;
          indexInInputs = join->getIndexInInputsByComputationName(prevComputationName);

          if ((indexInInputs < 0) || (indexInInputs > join->getNumInputs())) {
              indexInInputs = join->getIndexInInputsByTypeName(curInputSetIdentifier->getDataType());
          }
          if ((indexInInputs < 0) || (indexInInputs > join->getNumInputs())) {
              indexInInputs = 0;
          }
          curInputSetIdentifier->setIndexInInputs(indexInInputs);

          if ((matchOrNot == false)||(join->getJoinType() != LocalJoin)) {
              if (join->getJoinType() != CrossProduct)
                  join->setJoinType(HashPartitionedJoin);
              Handle<TupleSetJobStage> joinPrepStage = createTupleSetJobStage(
                  jobStageId, sourceTupleSetName, targetTupleSetName, mySpecifier,
                  buildTheseTupleSets, "IntermediateData", curInputSetIdentifier,
                  nullptr, sink, false, true, false, isProbing, join->getJoinType(), myPolicy, true, false, 0,
                  false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);

	      if (join->getJoinType()==CrossProduct) {

                  joinPrepStage->setJoinType("CrossProduct");

              } else if (join->getJoinType()==HashPartitionedJoin) {

                  joinPrepStage->setJoinType("HashPartitionedJoin");

              } else if (join->getJoinType()==LocalJoin) {

                  joinPrepStage->setJoinType("LocalJoin");

              } else if (join->getJoinType()==BroadcastJoin) {

                  joinPrepStage->setJoinType("BroadcastJoin");

              } else {

                  joinPrepStage->setJoinType("UnknownJoinType");
              }

              joinPrepStage->setJoinTupleSourceOrNot(true);
              physicalPlanToOutput.push_back(joinPrepStage);
              interGlobalSets.push_back(sink);
              std::string myJoinSource = curSource->getOutputName();
              std::string lastOne =
                  buildTheseTupleSets[buildTheseTupleSets.size() - 1];
              buildTheseTupleSets.clear();
              outputForJoinSets.push_back(outputName);
              buildTheseTupleSets.push_back(lastOne);
              buildTheseTupleSets.push_back(curNode->getOutputName());
              // Now I am the source!
              return analyze(physicalPlanToOutput, interGlobalSets,
                         buildTheseTupleSets, curNode, myComputation, sink,
                         nextNode, jobStageId, curNode, true, join->getJoinType(), myPolicy,
                         myJoinSource, prevSpecifier, false);
              // we then create a pipeline stage to probe the partitioned hash table

          } else {
              // we are pre-partitioned, now we only need to simply probe the partitioned
              // hash map built by hash partitioned join or a local join
              /*join->setJoinType(LocalJoin);
              if ((sourceComputation->getComputationType() == "ScanUserSet") ||
                 (sourceComputation->getComputationType() == "ScanSet")) {
                     Handle<ScanUserSet<Object>> scanner =
                        unsafeCast<ScanUserSet<Object>, Computation>(sourceComputation);
                     scanner->setFollowedByLocalJoin(true);
              }*/


              std::string outputName = joinNode->getOutputName();
              //std::string partitionedHashSetToProbe = (*hashSetsToProbe)[outputName];
              //(*hashSetsToProbe)[outputName] = partitionedHashSetToProbe;

              buildTheseTupleSets.push_back(curNode->getOutputName());
              outputForJoinSets.push_back(outputName);
              return analyze(physicalPlanToOutput, interGlobalSets,
                         buildTheseTupleSets, curSource, sourceComputation,
                         curInputSetIdentifier, nextNode, jobStageId, curNode,
                         true, join->getJoinType(), myPolicy, joinSource, prevSpecifier, true, partitionComputationName, partitionLambdaName);

          }

        } else {
          // we probe the broadcasted hash table
          // if my other input has been processed, I am not a pipeline breaker,
          // but we
          // should set the correct hash set names for probing
          buildTheseTupleSets.push_back(curNode->getOutputName());
          outputForJoinSets.push_back(outputName);

          return analyze(physicalPlanToOutput, interGlobalSets,
                         buildTheseTupleSets, curSource, sourceComputation,
                         curInputSetIdentifier, nextNode, jobStageId, curNode,
                         true, joinType, myPolicy, joinSource, prevSpecifier, hasLocalJoinProbe);
        }
      }

    } else {
      // I am not a pipeline breaker
      buildTheseTupleSets.push_back(curNode->getOutputName());
      return analyze(physicalPlanToOutput, interGlobalSets, buildTheseTupleSets,
                     curSource, sourceComputation, curInputSetIdentifier,
                     nextNode, jobStageId, curNode, isProbing, joinType, myPolicy,
                     joinSource, prevSpecifier, hasLocalJoinProbe);
    }

  } else {
    // I am a pipeline breaker because I have more than one consumers
    // TODO: check whether I am the final operator in a computation. If I am not
    // the final
    // operator in a computation, there is an error.
    Handle<SetIdentifier> sink = nullptr;
    if (myComputation->needsMaterializeOutput() == false) {
      myComputation->setOutput(this->jobId, outputName);
      sink = makeObject<SetIdentifier>(this->jobId, outputName);
      sink->setPageSize(conf->getPageSize());
      size_t desiredSize = this->exactSizeOfCurSource/conf->getPageSize()/this->numNodesInCluster;
      if (desiredSize == 0) {
          desiredSize = 1;
      }
      sink->setDesiredSize(desiredSize);
      interGlobalSets.push_back(sink);
    } else {
      // to get my output set
      std::string dbName = myComputation->getDatabaseName();
      std::string setName = myComputation->getSetName();
      sink = makeObject<SetIdentifier>(dbName, setName);
    }
    if (myComputation->getComputationType() == "JoinComp") {
      
      std::cout << "ERROR: We currently do not support JoinComp to have two consumers, please add an identity selection after the join" << std::endl;
      return false;
    }
    if (myComputation->getComputationType() != "ClusterAggregationComp") {
      buildTheseTupleSets.push_back(curNode->getOutputName());
      std::string sourceTupleSetName = curSource->getOutputName();
      if (joinSource != "") {
        sourceTupleSetName = joinSource;
        joinSource = "";
      }
      Handle<TupleSetJobStage> jobStage = createTupleSetJobStage(
          jobStageId, sourceTupleSetName, curNode->getOutputName(), mySpecifier,
          buildTheseTupleSets, myComputation->getOutputType(),
          curInputSetIdentifier, nullptr, sink, false, false, false, isProbing, joinType,
          myPolicy, false, false, 0, false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
      physicalPlanToOutput.push_back(jobStage);
    } else if (myComputation->getComputationType() ==
               "ClusterAggregationComp") {
      Handle<SetIdentifier> aggregator = makeObject<SetIdentifier>(
          this->jobId, outputName + "_aggregationData");
      aggregator->setPageSize(conf->getShufflePageSize());
      size_t desiredSize = this->exactSizeOfCurSource/conf->getShufflePageSize()/this->numNodesInCluster;
      if (desiredSize == 0) {
          desiredSize = 1;
      }
      aggregator->setDesiredSize(desiredSize);
      Handle<SetIdentifier> combiner = nullptr;
      if (myComputation->isUsingCombiner() == true) {
        combiner = makeObject<SetIdentifier>(this->jobId,
                                             outputName + "_combinerData");
      }
      std::string sourceTupleSetName = curSource->getOutputName();
      if (joinSource != "") {
        sourceTupleSetName = joinSource;
        joinSource = "";
      }
      Handle<TupleSetJobStage> jobStage = createTupleSetJobStage(
          jobStageId, sourceTupleSetName, curNode->getInputName(), mySpecifier,
          buildTheseTupleSets, "IntermediateData", curInputSetIdentifier,
          combiner, aggregator, false, true, false, isProbing, joinType, myPolicy, false,
          false, 0, false, hasLocalJoinProbe, partitionComputationSpecifier, partitionLambdaName);
      physicalPlanToOutput.push_back(jobStage);
      // to create the consuming job stage for aggregation
      Handle<AbstractAggregateComp> agg =
          unsafeCast<AbstractAggregateComp, Computation>(myComputation);
      Handle<AggregationJobStage> aggStage = createAggregationJobStage(
          jobStageId, agg, aggregator, sink, agg->getOutputType(),
          myComputation->needsMaterializeOutput());
      // to push back the aggregation stage;
      physicalPlanToOutput.push_back(aggStage);
      // to push back the aggregator set
      interGlobalSets.push_back(aggregator);

    } else {
      std::cout
          << "Computation Type: " << myComputation->getComputationType()
          << " are not supported to have more than one consumers right now"
          << std::endl;
      this->logger->fatal(
          "Computation Type: " + myComputation->getComputationType() +
          " are not supported to have more than one consumers right now");
      exit(1);
    }

    if (this->dynamicPlanningOrNot == true) {
      this->updateSourceSets(curInputSetIdentifier, sink, curNode);
      return true;

    } else {

      // now I am a source
      buildTheseTupleSets.clear();
      buildTheseTupleSets.push_back(curNode->getOutputName());
      for (int i = 0; i < numConsumersForCurNode; i++) {
        AtomicComputationPtr nextNode = consumers[i];
        //compare nextNode's computation and myComputation
        std::string nextSpecifier = nextNode->getComputationName();
        std::string prevSpecifier = prevComputationName;
        if (nextSpecifier != mySpecifier) {
              prevSpecifier = mySpecifier;
        }
        bool ret = analyze(physicalPlanToOutput, interGlobalSets,
                           buildTheseTupleSets, curNode, myComputation, sink,
                           nextNode, jobStageId, curNode, false, joinType, defaultAllocator, "", prevSpecifier, false); 
        if (ret == false) {
          return false;
        }
      }
      return true;
    }
  }
}

// to get current source sets;
std::vector<std::string> &TCAPAnalyzer::getCurSourceSetNames() {
  return curSourceSetNames;
}

// to get the source specified by index
std::string TCAPAnalyzer::getSourceSetName(int index) {
  return curSourceSetNames[index];
}

// to get source set based on name
Handle<SetIdentifier> TCAPAnalyzer::getSourceSetIdentifier(std::string name) {
  if (curSourceSets.count(name) == 0) {
    return nullptr;
  } else {
    return curSourceSets[name];
  }
}

// to get source computation based on name
AtomicComputationPtr TCAPAnalyzer::getSourceComputation(std::string name) {
  if (curSourceNodes.count(name) == 0) {
    return nullptr;
  } else {
    return curSourceNodes[name][0];
  }
}

// to get number of sources
int TCAPAnalyzer::getNumSources() { return curSourceSetNames.size(); }

// to return the index of the best source
int TCAPAnalyzer::getBestSource(StatisticsPtr stats) {
  // use the cost model to return the index of the best source
  
  if (stats == 0) {
    return 0;
  } else {
    int bestIndexToReturn = 0;
    double minCost = DBL_MAX;
    for (size_t i = 0; i < curSourceSetNames.size(); i++) {
      size_t curSize = getCostOfSource(i, stats);
      double curCost = double(curSize / 1000000);
      if (curCost < 1) {
          curCost = 1;
      }
      for (size_t j = 0; j < penalizedSourceSets.size(); j++) {
        if (curSourceSetNames[i] == penalizedSourceSets[j]) {
          if (stats->getPenalizedCost(curSourceSetNames[i]) > 0) {
              curCost = stats->getPenalizedCost(curSourceSetNames[i]);
          }
          curCost = curCost * 1000;
          stats->setPenalizedCost(curSourceSetNames[i], curCost);
          std::cout << "Meet a penalized source set: " << curSourceSetNames[i]
                    << ", we increase its cost by 1000 times to be " << curCost
                    << std::endl;
          break;
        }
      }
      if (curCost < minCost) {
        minCost = curCost;
        this->exactSizeOfCurSource = curSize;
        bestIndexToReturn = i;
      }
      // below is optimization for nearest neighbor search
      if (JOIN_HASH_TABLE_SIZE_RATIO > 1.5) {
        if (curCost == minCost) {
          minCost = curCost;
          bestIndexToReturn = i;
        }
      }
    }
    std::cout << "The Best Source (cost= " << minCost << ") is "
              << bestIndexToReturn << ": "
              << curSourceSetNames[bestIndexToReturn] << std::endl;
    this->costOfCurSource = minCost;
    this->curSourceSetName = curSourceSetNames[bestIndexToReturn];
    return bestIndexToReturn;
  }
}

// to return the cost of the i-th source
size_t TCAPAnalyzer::getCostOfSource(int index, StatisticsPtr stats) {
  std::string key = curSourceSetNames[index];
  Handle<SetIdentifier> curSet = this->getSourceSetIdentifier(key);
  if (curSet == nullptr) {
    std::cout << "WARNING: there is no source set for key=" << key << std::endl;
    return 0;
  }
  size_t cost = stats->getNumBytes(curSet->getDatabase(), curSet->getSetName());
  return cost;
}

// to return the index of next consumer to process for a certain source
unsigned int TCAPAnalyzer::getNextConsumerIndex(std::string name) {
  return curProcessedConsumers[name];
}

// to increment the index of next consumer to process for a certain source
void TCAPAnalyzer::incrementConsumerIndex(std::string name) {
  curProcessedConsumers[name] = curProcessedConsumers[name] + 1;
}


std::pair<std::string, std::string> TCAPAnalyzer::getHashSource (AtomicComputationPtr curSource,
                                                     std::shared_ptr<ApplyJoin> joinNode,
                                                     LogicalPlanPtr logicalPlan) {


          //to determine which side is the current input: left or right
          std::string sourceAtt = (curSource->getOutput().getAtts())[0];
          //left projection att
          std::string leftAtt = (joinNode->getProjection().getAtts())[0];
          //right projection att
          std::string rightAtt = (joinNode->getRightProjection().getAtts())[0];
          std::pair<std::string, std::string> res;
          if ((sourceAtt == leftAtt)||((sourceAtt.find("in")!=std::string::npos)&&(rightAtt.find("in")!=std::string::npos)&&(sourceAtt != rightAtt))) {
              AtomicComputationPtr producer = this->logicalPlan->getComputations().getProducingAtomicComputation(joinNode->getInput().getSetName());
              res = producer->findSource((joinNode->getInput().getAtts())[0],
                                                                     this->logicalPlan->getComputations());
          } else if ((sourceAtt == rightAtt)||((sourceAtt.find("in")!=std::string::npos)&&(leftAtt.find("in")!=std::string::npos)&&(sourceAtt != leftAtt))) {

              AtomicComputationPtr producer = this->logicalPlan->getComputations().getProducingAtomicComputation(joinNode->getRightInput().getSetName());
              res = producer->findSource((joinNode->getRightInput().getAtts())[0], this->logicalPlan->getComputations());
          } 


          return res;

}


bool TCAPAnalyzer::matchSourceWithQuery(std::string jobInstanceId, 
                            AtomicComputationPtr curSource,
                            std::shared_ptr<ApplyJoin> joinNode,
                            LogicalPlanPtr logicalPlan,
                            Handle<SetIdentifier> sourceSetIdentifier,
                            std::string & computationName,
                            std::string & lambdaName) {


          bool prepartitioned = false;
          std::pair<std::string, std::string> res = getHashSource(curSource, joinNode, logicalPlan);
          if ((res.first == "") && (res.second == "")) {
              return false;
          }
          std::string origJobName = this->db->getJobName(jobInstanceId);
          //we ignore everything in jobName after # so that users can pack additional runtime information after #
          size_t pos = origJobName.find('#');
          std::string jobName = origJobName.substr(0, pos);
          Handle<Vector<Handle<LambdaIdentifier>>> partitionLambdas =
            this->db->getPartitionLambdas(sourceSetIdentifier->getDatabase(),
                                         sourceSetIdentifier->getSetName());

          for (int i = 0; i < partitionLambdas->size(); i++) {

	      Handle<LambdaIdentifier> partitionLambda = (*partitionLambdas)[i];

              computationName = res.first;
              lambdaName = res.second;
              if ((partitionLambda->getJobName() == jobName) &&
                  (partitionLambda->getComputationName() == res.first) &&
                  (partitionLambda->getLambdaName() == res.second)) {

                  prepartitioned = true;


              }

              if (prepartitioned == false) {

                  Handle<LambdaIdentifier> queryLambda =
                    this->db->getLambda(jobName, res.first, res.second);

                  if ((partitionLambda->getLambdaIdentifier() == queryLambda->getLambdaIdentifier())                      && (partitionLambda->getLambdaInputClass() == queryLambda->getLambdaInputClass())) {

                     prepartitioned = true;

                  }

               }
               if (prepartitioned == true) 
                   break;

            }
            return prepartitioned;

}

}

#endif
