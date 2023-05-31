#ifndef TCAP_ANALYZER_H
#define TCAP_ANALYZER_H

#include "DataTypes.h"
#include "AggregationJobStage.h"
#include "AtomicComputationList.h"
#include "BroadcastJoinBuildHTJobStage.h"
#include "Computation.h"
#include "ComputePlan.h"
#include "Configuration.h"
#include "HashPartitionedJoinBuildHTJobStage.h"
#include "LogicalPlan.h"
#include "PDBLogger.h"
#include "Statistics.h"
#include "TupleSetJobStage.h"
#include "SelfLearningDB.h"

namespace pdb {


// This class encapsulates the analyzer for TCAP string
// You can use this class transform a TCAP string into a physical plan: a
// sequence of
// AbstractJobStage instances
// This class must be used in the same allocation block with the invoker of
// method analyze() to
// avoid problematic shallow copy.

//it analyzes the TCAP program as a DAG based on a cost model using a greedy algorithm.
//The goal is to minimize the volume of intermediate data.
//The approach is to monitor and collect storage statistics at runtime and select a source
//that has the lowest cost, and then go through the DAG from the source until it meets a pipeline breaker.

//Then it generates the corresponding JobStages:
// -- TupleSetJobStage: a pipeline
// -- AggregationJobStage: to perform the final aggregation on shuffled data
// -- BroadcastJoinBuildHTJobStage: to perform hash table building for broadcast join
// -- HashPartitionedJoinBuildHTJobStage: to perform hash table building for partitioned join



class TCAPAnalyzer {

public:
  // constructor
  TCAPAnalyzer(std::string jobId,
               Handle<Vector<Handle<Computation>>> myComputations,
               std::string myTCAPString, PDBLoggerPtr logger,
               ConfigurationPtr conf, std::shared_ptr<SelfLearningDB> db = nullptr, 
               bool isDynamicPlanning = false);

  // destructor
  ~TCAPAnalyzer();

  // to analyze the TCAP strings and get physical plan
  bool analyze(std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
               std::vector<Handle<SetIdentifier>> &interGlobalSets);

  // to analyze the subgraph rooted at a source node (sourceComputation) and get
  // a partial phyical
  // plan
  // if current node has two inputs, we need to specify the prev node
  bool analyze(std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
               std::vector<Handle<SetIdentifier>> &interGlobalSets,
               AtomicComputationPtr sourceComputation, int &jobStageId);

  // to analyze the subgraph rooted at a source node and only returns a set of
  // job stages
  // corresponding with the subgraph
  bool
  getNextStages(std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
                std::vector<Handle<SetIdentifier>> &interGlobalSets,
                AtomicComputationPtr curSource,
                Handle<SetIdentifier> curInputSetIdentifier, int &jobStageId);

  bool getNextStagesOptimized(
      std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
      std::vector<Handle<SetIdentifier>> &interGlobalSets,
      AtomicComputationPtr curSource,
      Handle<SetIdentifier> curInputSetIdentifier,
      unsigned int curConsumerIndex, int &jobStageId);

  // to create tuple set job stage
  Handle<TupleSetJobStage> createTupleSetJobStage(
      int &jobStageId, std::string sourceTupleSetName,
      std::string targetTupleSetName, std::string targetComputationName,
      std::vector<std::string> buildTheseTupleSets, std::string outputTypeName,
      Handle<SetIdentifier> sourceContext,
      Handle<SetIdentifier> combinerContext, Handle<SetIdentifier> sinkContext,
      bool isBroadcasting, bool isRepartitioning, bool needsRemoveInputSet,
      bool isProbing = false, JoinType joinType=UnknownJoin, AllocatorPolicy policy = defaultAllocator,
      bool isRepartitionJoin = false, bool isCollectAsMap = false,
      int numNodesToCollect = 0, bool hasLocalJoinSink = false, bool hasLocalJoinProbe = false, std::string partitionComputationName = "", std::string partitionLambdaName = "");

  // to create broadcast join stage
  Handle<BroadcastJoinBuildHTJobStage> createBroadcastJoinBuildHTJobStage(
      int &jobStageId, std::string sourceTupleSetName,
      std::string targetTupleSetName, std::string targetComputationName,
      Handle<SetIdentifier> sourceContext, std::string hashSetName,
      bool needsRemoveInputSet);

  // to create hash partitioned join stage
  Handle<HashPartitionedJoinBuildHTJobStage>
  createHashPartitionedJoinBuildHTJobStage(int &jobStageId,
                                           std::string sourceTupleSetName,
                                           std::string targetTupleSetName,
                                           std::string targetComputationName,
                                           Handle<SetIdentifier> sourceContext,
                                           std::string hashSetName,
                                           bool needsRemoveInputSet);

  // to create aggregation job stage
  Handle<AggregationJobStage> createAggregationJobStage(
      int &jobStageId, Handle<AbstractAggregateComp> aggComp,
      Handle<SetIdentifier> sourceContext, Handle<SetIdentifier> sinkContext,
      std::string outputTypeName, bool materializeOrNot);

  // to analyze subgraph rooted at any node (curNode) and get a physical plan
  bool analyze(std::vector<Handle<AbstractJobStage>> &physicalPlanToOutput,
               std::vector<Handle<SetIdentifier>> &interGlobalSets,
               std::vector<std::string> &buildTheseTupleSets,
               AtomicComputationPtr curSource,
               Handle<Computation> sourceComputation,
               Handle<SetIdentifier> curInputSetIdentifier,
               AtomicComputationPtr curNode, int &jobStageId,
               AtomicComputationPtr prevNode = nullptr,
               bool isProbing = false,
	       JoinType joinType = UnknownJoin,
               AllocatorPolicy policy = defaultAllocator,
               std::string joinSource = "",
               std::string prevComputationName = "",
               bool hasLocalJoinProbe = false,
               std::string partitionComputationSpecifier = "",
               std::string partitionLambdaName = "");

  // to get current source sets;
  std::vector<std::string> &getCurSourceSetNames();

  // to get the source specified by index
  std::string getSourceSetName(int index);

  // to get source set based on name
  Handle<SetIdentifier> getSourceSetIdentifier(std::string name);

  // to get source computation based on name
  AtomicComputationPtr getSourceComputation(std::string name);

  // to update source set names
  bool updateSourceSets(Handle<SetIdentifier> oldSet,
                        Handle<SetIdentifier> newSet,
                        AtomicComputationPtr newAtomicComp);

  // to remove source
  bool removeSource(std::string name);

  // to get number of sources
  int getNumSources();

  // to return the index of the best source
  int getBestSource(StatisticsPtr stats);

  // to return the cost of the i-th source
  size_t getCostOfSource(int index, StatisticsPtr stats);

  // to return the index of next consumer to process for a certain source
  unsigned int getNextConsumerIndex(std::string name);

  // to increment the index of next consumer to process for a certain source
  void incrementConsumerIndex(std::string name);

  // to get number of consumers for a certain source
  unsigned int getNumConsumers(std::string name);

  // to get the computation name and lambda name
  std::pair<std::string, std::string> getHashSource (AtomicComputationPtr curSource,
                                                     std::shared_ptr<ApplyJoin> joinNode,
                                                     LogicalPlanPtr logicalPlan);                                                     

  // to check whether the source partition lambda matches query lambda
  bool matchSourceWithQuery(std::string jobInstanceId, 
                            AtomicComputationPtr curSource,
                            std::shared_ptr<ApplyJoin> joinNode,
                            LogicalPlanPtr logicalPlan,
                            Handle<SetIdentifier> sourceSetIdentifier,
                            std::string & computationName,
                            std::string & lambdaName);



  // to set number of nodes
  void setNumNodes (int numNodes) {
      this->numNodesInCluster = numNodes;
  }

  // to return number of nodes
  int getNumNodes () {
      return this->numNodesInCluster;
  }

private:
  //selfLearning db
  std::shared_ptr<SelfLearningDB> db = nullptr;


  // hash sets to probe in current stage
  // needs to be cleared after execution of each stage
  Handle<Map<String, String>> hashSetsToProbe;

  // output for current joinSets operation
  std::vector<String> outputForJoinSets;

  // input computations
  Handle<Vector<Handle<Computation>>> computations;

  // input tcap string
  std::string tcapString;

  // compute plan generated from input computations and the input tcap string
  Handle<ComputePlan> computePlan;

  // logical plan generated from the compute plan
  LogicalPlanPtr logicalPlan;

  // the computation graph generated from the logical plan
  AtomicComputationList computationGraph;

  // the source nodes of logical plan, which will not change
  std::vector<AtomicComputationPtr> sources;

  // the logger
  PDBLoggerPtr logger;

  // the config
  ConfigurationPtr conf;

  // the jobId for this query
  std::string jobId;

  // whether to analyze the logical plan dynamically
  bool dynamicPlanningOrNot;

  // the mapping of set name to source set identifiers, which will only be used
  // in dynamicPlanning
  // mode, and will keep changing
  std::map<std::string, Handle<SetIdentifier>> curSourceSets;

  // the mapping of set name to source computation node, which will only be used
  // in
  // dynamicPlanning mode, and will keep changing
  std::map<std::string, std::vector<AtomicComputationPtr>> curSourceNodes;

  // the mapping of set name to number of processed consumers
  std::map<std::string, unsigned int> curProcessedConsumers;

  // the vector of set names
  std::vector<std::string> curSourceSetNames;

  // the size of current source
  double costOfCurSource;

  // the exact size of current source
  size_t exactSizeOfCurSource;


  // the name of current chosen source set
  std::string curSourceSetName;

  // the names of source sets that are hold on
  std::vector<std::string> penalizedSourceSets;

  // number of nodes in the cluster
  int numNodesInCluster;

};


}

#endif
