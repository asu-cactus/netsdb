#ifndef SELF_LEARNING_WRAPPER_SERVER_CC
#define SELF_LEARNING_WRAPPER_SERVER_CC

#include "SelfLearningWrapperServer.h"
#include "SelfLearningServer.h"
namespace pdb {


void SelfLearningWrapperServer :: cleanup() {}

void SelfLearningWrapperServer :: registerHandlers(PDBServer& forMe) {}


bool SelfLearningWrapperServer :: createLambda (long jobId, std::string lambdaType, std::string lambdaIdentifier, std::string computationName, std::string lambdaName, int indexInInputs, std::string lambdaInputClass, std::string lambdaOutputClass, long& id) {

    return getFunctionality<SelfLearningServer>().createLambda (jobId, lambdaType, lambdaIdentifier, computationName, lambdaName, indexInInputs, lambdaInputClass, lambdaOutputClass, id);
}


    

bool SelfLearningWrapperServer :: getProducerConsumerInfoForLoadJob(std::string loadJobName, LoadJobSelfLearningInfo& loadJob, std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>, PairKeyHash, PairKeyEqual>& consumers) {

    return getFunctionality<SelfLearningServer>().getProducerConsumerInfoForLoadJob(loadJobName, loadJob, consumers);

}

bool SelfLearningWrapperServer :: getAllLambdaCandidates(std::string loadJobName, std::string dataType, std::vector<Handle<LambdaIdentifier>> & lambdaCandidates, RLState & state) {

    return getFunctionality<SelfLearningServer>().getAllLambdaCandidates(loadJobName, dataType, lambdaCandidates, state);

}

Handle<LambdaIdentifier> SelfLearningWrapperServer :: getLambda(long jobId, std::string computationName, std::string inputType, int indexInInputs) {

   return getFunctionality<SelfLearningServer>().getLambda(jobId, computationName, inputType, indexInInputs);

}

long SelfLearningWrapperServer :: getSinkDataId (long jobStageId) {

   return getFunctionality<SelfLearningServer>().getSinkDataId(jobStageId);

}

bool SelfLearningWrapperServer :: getInfoForLoadJob (std::string loadJobName, double & average, double & variance) {
 
   return getFunctionality<SelfLearningServer>().getInfoForLoadJob(loadJobName, average, variance);

}


bool SelfLearningWrapperServer :: getConsumerInfoForData(long dataId,
                  std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>,
                  PairKeyHash, PairKeyEqual>& consumers) {

   return getFunctionality<SelfLearningServer>().getConsumerInfoForData(dataId, consumers);

}

std::string SelfLearningWrapperServer :: getDataType (long dataId) {

   return getFunctionality<SelfLearningServer>().getDataType(dataId);

}

bool SelfLearningWrapperServer :: getReward(std::string loadJobName, double &reward) {
   return getFunctionality<SelfLearningServer>().getReward(loadJobName, reward);
}

}

#endif
