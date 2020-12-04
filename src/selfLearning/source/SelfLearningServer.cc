#ifndef SELF_LEARNING_SERVER_CC
#define SELF_LEARNING_SERVER_CC

#include "SelfLearningServer.h"
#include "LambdaContext.h"
#include "JobStageContext.h"
#include "JobInstanceContext.h"
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <string>


namespace pdb {

SelfLearningServer :: ~SelfLearningServer() {}


SelfLearningServer :: SelfLearningServer(ConfigurationPtr conf, int port) {

    this->port = port;
    this->db = std::make_shared<SelfLearningDB>(conf);
}

std::shared_ptr<SelfLearningDB> SelfLearningServer::getDB() {

    return this->db;

}

void SelfLearningServer :: cleanup() {}

void SelfLearningServer :: registerHandlers(PDBServer& forMe) {}


bool SelfLearningServer :: createDir() {

     return this->db->createDir();

}




bool SelfLearningServer :: openDB() {

    return this->db->openDB();

}

bool SelfLearningServer :: execDB(std::string cmdString) {

    return this->db->execDB(cmdString);

}

bool SelfLearningServer :: createTables() {

    return this->db->createTables();
         
}

bool SelfLearningServer :: createData (std::string databaseName, std::string setName, std::string created_jobId,
                std::string setType, std::string className, int typeId, size_t pageSize, long lambdaId,
                int replicationFactor, long& id, long lambdaId1) {

    return this->db->createData(databaseName, setName, created_jobId, setType, className, typeId, pageSize, lambdaId,
                replicationFactor, id, lambdaId1);

}

bool SelfLearningServer :: updateDataForSize (long id, size_t size) {

    return this->db->updateDataForSize(id, size);


}

bool SelfLearningServer :: updateJobForInitialLatency (long id, double initialLatency, long initialJobInstanceId) {

    return this->db->updateJobForInitialLatency(id, initialLatency, initialJobInstanceId);

}


bool SelfLearningServer :: updateDataForRemoval (long id) {

    return this->db->updateDataForRemoval(id);

}

bool SelfLearningServer :: createJob (std::string jobName, std::string tcap, Handle<Vector<Handle<Computation>>> computations, 
                long& id) {

    return this->db->createJob(jobName, tcap, computations, id);

}

bool SelfLearningServer :: existsJob (std::string jobName, long& id) {

    return this->db->existsJob(jobName, id);

}

long SelfLearningServer::getLatestId (std::string tableName) {

    return this->db->getLatestId(tableName);

}

long SelfLearningServer::getLatestDataId () {

    return this->db->getLatestDataId();

}

long SelfLearningServer::getLatestDataIdForLoadJob (std::string loadJobName) {

    return this->db->getLatestDataIdForLoadJob(loadJobName);

}


long SelfLearningServer::getLatestJobId () {
     return getLatestId("JOB");
}

long SelfLearningServer::getLatestJobInstanceId () {
     return getLatestId("JOB_INSTANCE");
}

long SelfLearningServer::getLatestJobStageId () {
     return getLatestId("JOB_STAGE");
}

long SelfLearningServer::getLatestDataJobStageId () {
     return getLatestId("DATA_JOB_STAGE");
}

long SelfLearningServer::getLatestLambdaId () {
     return getLatestId("LAMBDA");
}


//below code is an example for how to retrive computations
/* Record<Vector<Handle<Computation>>> * retrievedRecord = getComputations(id);
   Handle<Vector<Handle<Computation>>> retrievedComputations = 
   retrievedRecord->getRootObject();
   std::cout << "Retrieved " << retrievedComputations->size() << 
                " Computations" << std::endl;
   free (retrievedRecord);
 */
//the user is responsible for free the returned memory 
Record<Vector<Handle<Computation>>> * SelfLearningServer :: getComputations (long id) {

     return this->db->getComputations(id);

}

Handle<Computation> SelfLearningServer :: getComputation(std::string jobName, std::string computationName) {

     return this->db->getComputation(jobName, computationName);

}


bool SelfLearningServer :: createJobInstance (long jobId, std::string jobIdForInstance, long& id) {


     return this->db->createJobInstance(jobId, jobIdForInstance, id);

}

bool SelfLearningServer :: updateJobInstanceForCompletion (long id, std::string status) {

     return this->db->updateJobInstanceForCompletion(id, status);

}

bool SelfLearningServer :: createJobStage (long jobInstanceId, int jobStageId, 
                std::string stageType, std::string status,
                std::string sourceType, std::string sinkType, std::string probeType,
                Handle<Vector<String>> buildTheseTupleSets, int numPartitions, 
                std::string targetComputationSpecifier, 
                Handle<Computation> aggregationComputation,
                long& id) {

     return this->db->createJobStage(jobInstanceId, jobStageId, stageType, status,
                                     sourceType, sinkType, probeType,
                                     buildTheseTupleSets, numPartitions,
                                     targetComputationSpecifier, aggregationComputation,
                                     id);


}


bool SelfLearningServer :: updateJobStageForCompletion (long id, std::string status) {

     return this->db->updateJobStageForCompletion(id, status);

}

bool SelfLearningServer :: updateJobStageForKeyDistribution (long id, int numHashKeys) {

     return this->db->updateJobStageForKeyDistribution(id, numHashKeys);

}


bool SelfLearningServer :: createDataJobStageMapping (long dataId, long jobInstanceStageId, int indexInInputs, std::string type, long& id) {

     return this->db->createDataJobStageMapping(dataId, jobInstanceStageId, indexInInputs, type, id);


}

bool SelfLearningServer :: createLambda (long jobId, 
                                         std::string lambdaType,
                                         std::string lambdaIdentifier,
                                         std::string computationName, 
                                         std::string lambdaName, 
                                         int lambdaInputIndexInComputationInputs, 
                                         std::string lambdaInputClass,
                                         std::string lambdaOutputClass, 
                                         long& id) {


     return this->db->createLambda(jobId, lambdaType, lambdaIdentifier, computationName, lambdaName, 
                                   lambdaInputIndexInComputationInputs, 
                                   lambdaInputClass, lambdaOutputClass, id);


}

bool SelfLearningServer :: getInfoForLoadJob (std::string loadJobName, double & average, double & variance) {


     return this->db->getInfoForLoadJob(loadJobName, average, variance);

}


bool SelfLearningServer :: getAllLambdaCandidates(std::string loadJobName, std::string dataType, std::vector<Handle<LambdaIdentifier>> & lambdaCandidates, RLState & state) {
    

     return this->db->getAllLambdaCandidates(loadJobName, dataType, lambdaCandidates, state);

}


bool SelfLearningServer :: getProducerConsumerInfoForLoadJob(std::string loadJobName, LoadJobSelfLearningInfo& loadJob, std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>, PairKeyHash, PairKeyEqual>& consumers) {


     return this->db->getProducerConsumerInfoForLoadJob(loadJobName,  loadJob, consumers);

}

Handle<LambdaIdentifier> SelfLearningServer :: getLambda(long jobId, std::string computationName, 
       std::string inputType, int indexInInputs) {

     return this->db->getLambda(jobId, computationName, inputType, indexInInputs);

}


Handle<LambdaIdentifier> SelfLearningServer :: getPartitionLambda(std::string databaseName, std::string setName) {

     return this->db->getPartitionLambda(databaseName, setName);

}

bool SelfLearningServer::getReward(std::string loadJobName, double &reward) {

     return this->db->getReward(loadJobName, reward); 

}

bool SelfLearningServer::getSinkData (long jobStageId, std::pair<std::string, std::string>& data) {

     return this->db->getSinkData(jobStageId, data);
     
}

long SelfLearningServer::getJobId (std::string jobName) {

   return this->db->getJobId(jobName);

}

long SelfLearningServer::getLambdaId (std::string jobName, std::string computationName, std::string lambdaName) {

   return this->db->getLambdaId(jobName, computationName, lambdaName);

}


long SelfLearningServer::getSinkDataId (long jobStageId) {


   return this->db->getSinkDataId(jobStageId);

}

std::string SelfLearningServer::getDataType (long dataId) {

   return this->db->getDataType(dataId);

}

std::string SelfLearningServer::getJobName (long jobId) {

   return this->db->getJobName(jobId);

}

std::string SelfLearningServer::getJobName (std::string jobInstanceId) {

   return this->db->getJobName(jobInstanceId);

}

bool SelfLearningServer::getConsumerInfoForData(long dataId,
                  std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>,
                  PairKeyHash, PairKeyEqual>& consumers) {

    return this->db->getConsumerInfoForData(dataId, consumers);
}


long SelfLearningServer::getLatestLambdaIdForLoadJob (std::string loadJobName) {

    long dataId = this->db->getLatestDataIdForLoadJob(loadJobName);
    long lambdaId = this->db->getLambdaId(dataId);
    return lambdaId;

}


}



#endif
