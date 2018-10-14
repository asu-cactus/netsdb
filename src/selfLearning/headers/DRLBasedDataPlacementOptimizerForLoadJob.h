#ifndef DRL_BASED_DATA_PLACEMENT_OPTIMIZER_FOR_LOAD_JOB_H
#define DRL_BASED_DATA_PLACEMENT_OPTIMIZER_FOR_LOAD_JOB_H


#include "LambdaIdentifier.h"
#include "LoadJobSelfLearningInfo.h"
#include "JobStageSelfLearningInfo.h"
#include "SelfLearningWrapperServer.h"
#include "AbstractDataPlacementOptimizer.h"
#include "RLClient.h"
#include "RLState.h"
#include <unordered_map>

namespace pdb {

class DRLBasedDataPlacementOptimizerForLoadJob : public AbstractDataPlacementOptimizer {

public:

//constructor
DRLBasedDataPlacementOptimizerForLoadJob(int numTotalPartitions,
                                          int numNodes, 
                                          std::string loadJobName, 
                                          std::string dataType, 
                                          SelfLearningWrapperServer& server,
                                          std::shared_ptr<RLClient> rlClientPtr);


//to return the best lambda
Handle<LambdaIdentifier> getBestLambda() override;

//to return the best page size
size_t getBestPageSize() override;

//to return the last reward
double getLastReward() override;

//to get all candidate lambdas and for each lambda, we compute its:

// (1) distance to current dataset
// ---- if one candidate is the direct consumer of the data to generate, distance is 0
// ---- if one candidate is the consumer of a direct consumer, distance is 1, and so on
// ---- if one candidate is not any consumer of the data directly or indirectly, the distance is -1
// ---- if one candidate has more than two distances, we set it to be the smallest positive one

// (2) frequency of the lambda
// ---- if the lambda appears in one consumer stage as the partition lambda or part of the partition lambda, we add 1 to the count

// (3) selectivity of the lambda
// ---- in history, the ratio of the output size of the computation stage that contains the output lambda to the size of the data to be generated
// ---- if one candidate has more than two selectivities, we set it to be the smallest positive one

bool populateLambdaCandidates(std::vector<Handle<LambdaIdentifier>> & lambdaCandidate, RLState & state);


//to return the estimated size of data to load
bool estimateSizeOfDataToLoad(double & avgSize, double & variance);


//to return the reward for applying last RL-selected lambda
bool computeLastReward(double& reward);



private:

//lambda candidates
std::vector<Handle<LambdaIdentifier>> lambdaCandidates;

//RLState
RLState state;

//last reward
double reward;

//total number of partitions 
int numPartitions = 0;

//producer job name
std::string producerJobName="";

//type of the data
std::string dataType = "";

//server to access self learning database
SelfLearningWrapperServer server;

//the client to talk to the RLServer
std::shared_ptr<RLClient> rlClientPtr = nullptr;

//best lambda index
int bestLambdaIndex = -1;

};

}

#endif
