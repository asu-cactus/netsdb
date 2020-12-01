#ifndef SELFLEARNING_DB_H
#define SELFLEARNING_DB_H

#include <sqlite3.h>
#include <memory>
#include "ServerFunctionality.h"
#include "JobStageSelfLearningInfo.h"
#include "LoadJobSelfLearningInfo.h"
#include "Handle.h"
#include "PDBVector.h"
#include "Configuration.h"
#include "Computation.h"
#include "LambdaIdentifier.h"
#include "PairKey.h"
#include "RLState.h"

namespace pdb {



// Manage a sqlite3 database for statistics.
class SelfLearningDB {

public:
    // destructor
    ~SelfLearningDB();

    // constructor, initialize from a sqlite database file
    SelfLearningDB( ConfigurationPtr conf );

    // open the database
    bool openDB();

    // create tables
    bool createTables();

    // query the database
    bool execDB(std::string cmdString);

    // create directories
    bool createDir();


    // methods for updating statistics
    // to update data information
    // --------------------------
    // --to create an entry in the DATA table each time when we create a set in distributed storage and each time when we 
    //schedule a BuildHashSet job stage or a BuildPartitionedHashSet job stage
    bool createData (std::string databaseName, std::string setName, std::string created_jobId,
                std::string setType, std::string className, int typeId, size_t pageSize, 
                long lambdaId, int replicationFactor, long& id, long lambdaId1=-1);

    // --to update the SIZE field and MODIFICATION_TIME field in the DATA table each time when we finish execution of a job stage, or each time we invoke flush
    bool updateDataForSize (long id, size_t size);


    // --to update the IS_REMOVED field and MODIFICATION_TIME field in the DATA table each time when we remove a dataset
    bool updateDataForRemoval (long id);


    // to update job information
    // -------------------------
    // --to query whether a job needs to be created and updated, and create/update an entry if needed in the JOB table each time when a client submits a Job
    bool createJob (std::string jobName, std::string tcap, Handle<Vector<Handle<Computation>>> computations, 
                long& id);


    //to check whether a job exists in the Job Table or not
    bool existsJob (std::string jobName, long& id);

    //to retrieve the computations corresponding to one job
    Record<Vector<Handle<Computation>>>* getComputations (long id); 

    //to retrieve the computation corresponding to specific job name and computation name
    Handle<Computation> getComputation(std::string jobName, std::string computationName);

    // to update job instance information
    // ----------------------------------
    // --to create an entry in the JOB_INSTANCE table each time when a client submits a Job
    bool createJobInstance (long jobId, std::string jobIdForInstance, long& id);


    // --to update the STATUS field and FINISH_TIME field in the JOB_INSTANCE table each time when we finish execution of a job
    bool updateJobInstanceForCompletion (long id, std::string status);


    // to update job stage information
    // -------------------------------
    // --to create an entry in the JOB_STAGE table each time when a JobStage is scheduled
    bool createJobStage (long jobInstanceId, int jobStageId, 
                std::string stageType, std::string status,
                std::string sourceType, std::string sinkType, std::string probeType,
                Handle<Vector<String>> buildTheseTupleSets, int numPartitions, 
                std::string targetComputationSpecifier, 
                Handle<Computation> aggregationComputation,
                long& id);


    // --to update the FINISH_TIME field in the JOB_STAGE table when we finish execution of a job stage
    bool updateJobStageForCompletion (long id, std::string status);

    // --to update the NUM_HASH_KEYS field in the JOB_STAGE table when we finish execution of a job stage
    bool updateJobStageForKeyDistribution (long id, int numHashKeys);

    // to update data-job information
    // ------------------------------
    // --to create an entry in the JOB_STAGE table each time when a JobStage is scheduled
    bool createDataJobStageMapping (long dataId, long jobInstanceStageId, int indexInInputs, 
                 std::string type, long& id);


    // to update lambda information
    // ----------------------------
    // --to query whether a lambda needs to be created, and create an entry if needed in the LAMBDA table each time when a Lambda is extracted, or a Lambda is traversed    
    bool createLambda (long jobId, 
                       std::string lambdaType, 
                       std::string lambdaIdentifier, 
                       std::string computationName, 
                       std::string lambdaName, 
                       int indexInInputs, 
                       std::string lambdaInputClass,
                       std::string lambdaOutputClass, 
                       long& id);



    //to get average and variance of output size for a specified load job
    bool getInfoForLoadJob (std::string loadJobName, double & average, double & variance);


    // to get a list of consumers of a load job
    // ---------------------------------------
    // --to query which customer stage will consume the output of this load job
    // --in the map, the key is a stage identifier that is a pair of jobId and jobStageId
    bool getProducerConsumerInfoForLoadJob(std::string loadJobName, LoadJobSelfLearningInfo& loadJob, 
                  std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>, 
                  PairKeyHash, PairKeyEqual>& consumers);  


    // to get all lambda candidates and their information
    bool getAllLambdaCandidates(std::string loadJobName, std::string dataType, std::vector<Handle<LambdaIdentifier>> & lambdaCandidates, RLState & state);

    // to get all lambda candidates and their information
    bool getAllLambdaCandidates1(std::string loadJobName, std::string dataType, std::vector<Handle<LambdaIdentifier>> & lambdaCandidates, RLState & state);


    // to get Lambda given a Computation and a type
    Handle<LambdaIdentifier> getLambda(long jobId, std::string computationName, std::string inputType, int indexInInputs);

    // to get Lambda given job name, computation name and lambda name
    Handle<LambdaIdentifier> getLambda(std::string jobName, std::string computationName, std::string lambdaName);

    // to get Lambda and Lambda1 given a set
    Handle<Vector<Handle<LambdaIdentifier>>> getPartitionLambdas(std::string databaseName, std::string setName);

    // to get partition Lambda given a set
    Handle<LambdaIdentifier> getPartitionLambda(std::string databaseName, std::string setName);

    // to get data size that is loaded by a given job and processed by another given job 
    size_t getDataSize (std::string loadJobName, long jobInstanceId);

    // to get reward
    bool getReward(std::string loadJobName, double &reward);

    // to update the Job table for initial latency field
    bool updateJobForInitialLatency (long id, double initialLatency, long initialJobInstanceId); 

    // to get the database name, set name of the sink data given a job stage id;
    bool getSinkData (long jobStageId, std::pair<std::string, std::string>& data);

    // to get the id of a sink data for a job stage;
    long getSinkDataId (long jobStageId);

    // to get the type of a data
    std::string getDataType (long dataId);    

    // to get the job name given a job Id
    std::string getJobName (long jobId);

    // to get the job name given a job instance Id
    std::string getJobName (std::string jobInstanceId);

    //to get the job id given a job name
    long getJobId (std::string jobName);

    // to get the id of the Lambda
    long getLambdaId (std::string jobName, std::string computationName, std::string lambdaName);

    // to get the id of the lambda
    long getLambdaId (long dataId);


    // to get a list of consumers of a data
    bool getConsumerInfoForData(long dataId,
                  std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>,
                  PairKeyHash, PairKeyEqual>& consumers);

    //to get latest id for a specified table
    long getLatestId (std::string tableName);

    //to get latest data id
    long getLatestDataId ();

    //to get latest data id that is created by a specified load job
    long getLatestDataIdForLoadJob (std::string loadJobName);

    //to get latest job id
    long getLatestJobId ();

    //to get latest job-instance id
    long getLatestJobInstanceId ();

    //to get latest job-stage id
    long getLatestJobStageId ();

    //to get latest data-job-stage id
    long getLatestDataJobStageId ();

    //to get latest lambda id
    long getLatestLambdaId ();




/*
    // to remove information before a certain timestamp
    bool removeAllEntriesByTime (time_t timestamp);


*/


protected:

    // to quote a string
    std::string quoteStr(std::string& s);

    // to find and replace a string
    void replaceStr(std::string& str,
               const std::string& oldStr,
               const std::string& newStr);

private:

    //sqlite database file
    std::string pathToDBFile;

    //Handler for the sqlite database instance
    sqlite3 * selfLearningDBHandler = nullptr;

    //data id
    long dataId;

    //job id
    long jobId;

    //job instance id
    long jobInstanceId;

    //job stage id
    long jobInstanceStageId;

    //lambda id
    long lambdaId;

    //id of data -> job instance mapping
    long dataToJobStageMappingId;

    //configuration
    ConfigurationPtr conf;



};

}


#endif
