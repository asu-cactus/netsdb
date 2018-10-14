#ifndef SELF_LEARNING_WRAPPER_SERVER_H
#define SELF_LEARNING_WRAPPER_SERVER_H

#include "ServerFunctionality.h"
#include "LoadJobSelfLearningInfo.h"
#include "JobStageSelfLearningInfo.h"
#include "LambdaIdentifier.h"
#include "RLState.h"
#include "PairKey.h"
#include <unordered_map>

// this wrapper is mainly added to solve the circular include dependency problems between the Lambda class and the SelfLearningServer class.

namespace pdb {

class SelfLearningWrapperServer : public ServerFunctionality {

public:

     //destructor
     ~SelfLearningWrapperServer() {}

     //constructor
     SelfLearningWrapperServer() {}

    // from the serverFunctionality interface... register the self-learning server handlers
    void registerHandlers(PDBServer& forMe) override;

    void cleanup() override;

   // to update lambda information (done by invoking function in SelfLearningServer)
    // ----------------------------
    // --to query whether a lambda needs to be created, and create an entry if needed in the LAMBDA table each time when a Lambda is extracted, or a Lambda is traversed    
    bool createLambda (long jobId, std::string lambdaType, std::string lambdaIdentifier, std::string computationName, std::string lambdaName, int indexInInputs, std::string lambdaInputClass, std::string lambdaOutputClass, long& id);

    //to get average and variance of output size for a specified load job
    bool getInfoForLoadJob (std::string loadJobName, double & average, double & variance);

    // to get a list of consumers of a load job
    // ---------------------------------------
    // --to query which customer stage will consume the output of this load job
    // --in the map, the key is a stage identifier that is a pair of jobId and jobStageId
    bool getProducerConsumerInfoForLoadJob(std::string loadJobName, LoadJobSelfLearningInfo& loadJob, std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>, PairKeyHash, PairKeyEqual>& consumers);

    // to get all lambda candidates and their information
    bool getAllLambdaCandidates(std::string loadJobName, std::string dataType, std::vector<Handle<LambdaIdentifier>> & lambdaCandidates, RLState & state);


    // to get Lambda given a Computation and a type
    Handle<LambdaIdentifier> getLambda(long jobId, std::string computationName, std::string inputType, int indexInInputs);

    // to get the id of a sink data;
    long getSinkDataId (long jobStageId);


    // to get a list of consumers of a data
    bool getConsumerInfoForData(long dataId,
                  std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>,
                  PairKeyHash, PairKeyEqual>& consumers);

    // to get the type of a data
    std::string getDataType (long dataId);
   
    // to get reward
    bool getReward(std::string loadJobName, double &reward);

private:
    long lambdaId = 0;

};

}


#endif

