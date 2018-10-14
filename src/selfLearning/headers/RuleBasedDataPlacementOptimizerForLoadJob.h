#ifndef RULE_BASED_DATA_PLACEMENT_OPTIMIZER_FOR_LOAD_JOB_H
#define RULE_BASED_DATA_PLACEMENT_OPTIMIZER_FOR_LOAD_JOB_H


#include "LambdaIdentifier.h"
#include "LoadJobSelfLearningInfo.h"
#include "JobStageSelfLearningInfo.h"
#include "SelfLearningWrapperServer.h"
#include "AbstractDataPlacementOptimizer.h"
#include <unordered_map>

namespace pdb {

class RuleBasedDataPlacementOptimizerForLoadJob : public AbstractDataPlacementOptimizer {

public:

//constructor
RuleBasedDataPlacementOptimizerForLoadJob(int numTotalPartitions, 
                                          std::string loadJobName, 
                                          std::string dataType, 
                                          SelfLearningWrapperServer& server);


//to return the best lambda
Handle<LambdaIdentifier> getBestLambda() override;

//to return the best page size
size_t getBestPageSize() override;


private:

//info of the producer
LoadJobSelfLearningInfo loadJobInfo;

//info of the consumers
std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>, PairKeyHash, PairKeyEqual> consumers;

//total number of partitions 
int numPartitions = 0;

//type of the data
std::string dataType = "";

//server to access self learning database
SelfLearningWrapperServer server;

};

}

#endif
