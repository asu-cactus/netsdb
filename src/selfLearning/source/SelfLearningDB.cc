#ifndef SELF_LEARNING_DB_CC
#define SELF_LEARNING_DB_CC

#include "SelfLearningDB.h"
#include "LambdaContext.h"
#include "JobStageContext.h"
#include "JobInstanceContext.h"
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <string>


namespace pdb {

SelfLearningDB :: ~SelfLearningDB() {}


SelfLearningDB :: SelfLearningDB(ConfigurationPtr conf) {

    this->conf = conf;
    this->pathToDBFile = "file:" + this->conf->getSelfLearningDB() + "/dbFile";

    //check whether pathToDBFile exists
    //if it exists, we do not need create directories and tables
    //otherwise, we need create directories and tables
    if (createDir()){
        createTables();
    } else {
        openDB();
    }

    //initialize ids
    dataId = getLatestDataId() + 1;
    jobId = getLatestJobId() + 1;
    jobInstanceId = getLatestJobInstanceId() + 1;
    jobInstanceStageId = getLatestJobStageId() + 1;
    lambdaId = getLatestLambdaId() + 1;
    dataToJobStageMappingId = getLatestDataJobStageId() + 1;

}




bool SelfLearningDB :: createDir() {

    if (mkdir(this->conf->getSelfLearningDB().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)!= -1){
        return true;
    } else {
        return false;
    }

}

std::string SelfLearningDB :: quoteStr(std::string& s) {

    return std::string("'") + s + std::string("'");

}

void SelfLearningDB :: replaceStr (std::string& str,
                                       const std::string& oldStr,
                                       const std::string& newStr)
{
    std::string::size_type pos = 0u;
    while((pos = str.find(oldStr, pos)) != std::string::npos){
       str.replace(pos, oldStr.length(), newStr);
       pos += newStr.length();
  }
}



bool SelfLearningDB :: openDB() {

    std::cout << "to open DB " << this->pathToDBFile << std::endl;
    if (sqlite3_open_v2(this->pathToDBFile.c_str(), &selfLearningDBHandler,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL) == SQLITE_OK) {

        sqlite3_exec(selfLearningDBHandler, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
        std::cout << "self learning database open successfully" << std::endl;
        return true;
    } else {
        std::cout << "failure in opening self learning database" << std::endl;
        return false;
    }

}

bool SelfLearningDB :: execDB(std::string cmdString) {

    std::cout << "command: " << cmdString << std::endl;
    sqlite3_stmt * cmdStatement;
    if (sqlite3_prepare_v2 (selfLearningDBHandler, cmdString.c_str(), -1, &cmdStatement,
                     NULL) == SQLITE_OK) {
        sqlite3_step(cmdStatement);
        sqlite3_finalize(cmdStatement);
        return true;
    } else {
        std::string error = sqlite3_errmsg(selfLearningDBHandler);
        std::cout << error << std::endl;
        sqlite3_finalize(cmdStatement);
        return false;
    }

}

bool SelfLearningDB :: createTables() {

    if (openDB() == false) {
        return false;
    }

    execDB ("CREATE TABLE IF NOT EXISTS DATA (ID BIGINT PRIMARY KEY, "
            "DATABASE_NAME VARCHAR(128), SET_NAME VARCHAR(128),"
            "CREATED_JOBID VARCHAR(128), IS_REMOVED BOOLEAN,"
            "SET_TYPE VARCHAR(32), CLASS_NAME VARCHAR(128),"
            "TYPE_ID INT, SIZE BIGINT, PAGE_SIZE BIGINT, LAMBDA_ID BIGINT,"
            "REPLICATION INT, MODIFICATION_TIME BIGINT, LAMBDA_ID1 BIGINT) WITHOUT ROWID;");

    execDB ("CREATE TABLE IF NOT EXISTS DATA_JOB_STAGE (ID BIGINT PRIMARY KEY, "
            "DATA_ID BIGINT, JOB_STAGE_ID BIGINT, INDEX_IN_INPUTS INT, DATA_TYPE VARCHAR(8),"
            "FOREIGN KEY (DATA_ID) REFERENCES DATA(ID),"
            "FOREIGN KEY (JOB_STAGE_ID) REFERENCES JOB_STAGE(ID)) WITHOUT ROWID;");

    execDB ("CREATE TABLE IF NOT EXISTS JOB (ID BIGINT PRIMARY KEY, "
            "NAME VARCHAR(128), TCAP TEXT, COMPUTATIONS BLOB, INITIAL_LATENCY FLOAT, INITIAL_JOB_INSTANCE_ID BIGINT) WITHOUT ROWID;");

    execDB ("CREATE TABLE IF NOT EXISTS JOB_INSTANCE (ID BIGINT PRIMARY KEY, "
            "JOB_ID BIGINT, JOB_INSTANCE_ID VARCHAR(128), STATUS VARCHAR(16),"
            "SUBMIT_TIME BIGINT, FINISH_TIME BIGINT,"
            "FOREIGN KEY (JOB_ID) REFERENCES JOB(ID)) WITHOUT ROWID;");

    execDB ("CREATE TABLE IF NOT EXISTS JOB_STAGE (ID BIGINT PRIMARY KEY, "
            "JOB_INSTANCE_ID BIGINT, STAGE_ID INTEGER, STAGE_TYPE VARCHAR(32), STATUS VARCHAR(16),"
            "SOURCE_TYPE VARCHAR(16), SINK_TYPE VARCHAR(16), PROBE_TYPE VARCHAR(32),"
            "TUPLESET_SPECIFIERS BLOB, NUM_HASH_KEYS INTEGER, NUM_PARTITIONS INTEGER,"
            "TARGET_COMPUTATION_SPECIFIER VARCHAR(128), AGG_COMPUTATION BLOB,"
            "START_TIME BIGINT, FINISH_TIME BIGINT,"
            "FOREIGN KEY (JOB_INSTANCE_ID) REFERENCES JOB_INSTANCE(ID)) WITHOUT ROWID;");

    execDB ("CREATE TABLE IF NOT EXISTS LAMBDA (ID BIGINT PRIMARY KEY, "
            "JOB_ID BIGINT, LAMBDA_TYPE VARCHAR(32), LAMBDA_IDENTIFIER VARCHAR(128), "
            "COMPUTATION_NAME VARCHAR(128), LAMBDA_NAME VARCHAR(128),"
            "LAMBDA_INPUT_INDEX_IN_COMPUTATION_INPUTS INTEGER,"
            "LAMBDA_INPUT_CLASS VARCHAR(128), LAMBDA_OUTPUT_CLASS VARCHAR(128),"
            "FOREIGN KEY (JOB_ID) REFERENCES JOB(ID)) WITHOUT ROWID;");

    return true;
         
}

bool SelfLearningDB :: createData (std::string databaseName, std::string setName, std::string created_jobId,
                std::string setType, std::string className, int typeId, size_t pageSize, long lambdaId,
                int replicationFactor, long& id, long lambdaId1) {
     std::cout << "to create data..." << std::endl;
     id = dataId;
     dataId ++;
     string cmdString = "INSERT INTO DATA "
                " (ID, DATABASE_NAME, SET_NAME, CREATED_JOBID, IS_REMOVED, SET_TYPE, "
                "CLASS_NAME, TYPE_ID, SIZE, PAGE_SIZE, LAMBDA_ID, REPLICATION, MODIFICATION_TIME, LAMBDA_ID1) "
                "VALUES(" + std::to_string(id) + "," 
                          + quoteStr(databaseName) + ","
                          + quoteStr(setName) + ","
                          + quoteStr(created_jobId) + ","
                          + "0,"
                          + quoteStr(setType) + ","
                          + quoteStr(className) + ","
                          + std::to_string(typeId) + ","
                          + "0,"
                          + std::to_string(pageSize) + ","
                          + std::to_string(lambdaId) + ","
                          + std::to_string(replicationFactor)+","
                          + "strftime('%s', 'now', 'localtime'),"
                          + std::to_string(lambdaId1)+"));";
      std::cout << "CreateData: " << cmdString << std::endl;
      return execDB(cmdString);

}

bool SelfLearningDB :: updateDataForSize (long id, size_t size) {
      std::string cmdString = "UPDATE DATA set SIZE = " + std::to_string(size) + 
                              ", MODIFICATION_TIME = strftime('%s', 'now', 'localtime') where ID = " +
                              std::to_string(id);
      std::cout << "UpdateDataForSize: " << cmdString << std::endl;
      return execDB(cmdString);

}

bool SelfLearningDB :: updateJobForInitialLatency (long id, double initialLatency, long initialJobInstanceId) {
      std::string cmdString = "UPDATE JOB set INITIAL_LATENCY = " + std::to_string(initialLatency) + ", INITIAL_JOB_INSTANCE_ID = " + std::to_string(initialJobInstanceId) +
                              " where ID = " +
                              std::to_string(id);
      std::cout << "UpdateJobForInitialLatency: " << cmdString << std::endl;
      return execDB(cmdString);
}


bool SelfLearningDB :: updateDataForRemoval (long id) {
      std::string cmdString = std::string("UPDATE DATA set IS_REMOVED = 1") + 
                              std::string(", MODIFICATION_TIME = strftime('%s', 'now', 'localtime') where ID = ") +
                              std::to_string(id);
      std::cout << "UpdateDataForRemoval: " << cmdString << std::endl;
      return execDB(cmdString);

}

bool SelfLearningDB :: createJob (std::string jobName, std::string tcap, Handle<Vector<Handle<Computation>>> computations, 
                long& id) {

      //first check whether a job exists or not
      if (existsJob(jobName, id)) {
           std::cout << "Job: " << jobName << " exists in the JOB table" << std::endl;
           return false;
      } else {
           id = jobId;
           jobId ++;
           sqlite3_stmt * statement;
           replaceStr(tcap, "'", "''");
           string cmdString = "INSERT INTO JOB "
                " (ID, NAME, TCAP, COMPUTATIONS, INITIAL_LATENCY, INITIAL_JOB_INSTANCE_ID) "
                "VALUES(" + std::to_string(id) + ","
                          + quoteStr(jobName) + ","
                          + quoteStr(tcap) + ","
                          + "?,-1.0, -1.0);";
           std::cout << "CreateJob: " << cmdString << std::endl;
           if (sqlite3_prepare_v2(selfLearningDBHandler, cmdString.c_str(),
             -1, &statement, NULL) == SQLITE_OK) {
                //to get the bytes
                Record<Vector<Handle<Computation>>> * record =
                     getRecord<Vector<Handle<Computation>>>(computations);
                sqlite3_bind_blob(statement, 1, record, record->numBytes(), SQLITE_STATIC);
                sqlite3_step(statement);
                sqlite3_finalize(statement);

                return true;
           } else {
                std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
                sqlite3_finalize(statement);
                return false;
           }

      }
}

bool SelfLearningDB :: existsJob (std::string jobName, long& id) {

      bool ret = false;
      sqlite3_stmt * statement;
      std::string queryString = "SELECT ID from JOB WHERE NAME = " + quoteStr(jobName);
      std::cout << "ExistsJob: " << queryString << std::endl;
      if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {

          int res = sqlite3_step(statement);
          if (res == SQLITE_ROW) {
              ret = true;
              id = sqlite3_column_int(statement, 0);
          } else {
              ret = false;
              id = -1;
          }

      } else {
          std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
          ret = false;
          id = -1;
      }
      sqlite3_finalize(statement);
      return ret;

}

long SelfLearningDB::getLatestId (std::string tableName) {

     long id;
     sqlite3_stmt * statement;
     std::string queryString = "SELECT MAX(ID) from " + tableName;
     if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement, NULL) == SQLITE_OK) {
         int res = sqlite3_step(statement);
         if (res == SQLITE_ROW) {
            id = sqlite3_column_int(statement, 0);
         } else {
            id = -1;
         }         
     } else {
         std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
         id = -1;
     } 
     sqlite3_finalize(statement);
     return id;
}

long SelfLearningDB::getLatestDataId () {
     return getLatestId("DATA");
}

long SelfLearningDB::getLatestDataIdForLoadJob (std::string loadJobName) {
     long id;
     sqlite3_stmt * statement;
     std::string queryString = "SELECT MAX(ID) from DATA WHERE CREATED_JOBID = " + quoteStr(loadJobName);
     if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement, NULL) == SQLITE_OK) {
         int res = sqlite3_step(statement);
         if (res == SQLITE_ROW) {
            id = sqlite3_column_int(statement, 0);
         } else {
            id = -1;
         }
     } else {
         std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
         id = -1;
     }
     sqlite3_finalize(statement);
     return id;
}


long SelfLearningDB::getLatestJobId () {
     return getLatestId("JOB");
}

long SelfLearningDB::getLatestJobInstanceId () {
     return getLatestId("JOB_INSTANCE");
}

long SelfLearningDB::getLatestJobStageId () {
     return getLatestId("JOB_STAGE");
}

long SelfLearningDB::getLatestDataJobStageId () {
     return getLatestId("DATA_JOB_STAGE");
}

long SelfLearningDB::getLatestLambdaId () {
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
Record<Vector<Handle<Computation>>> * SelfLearningDB :: getComputations (long id) {

      Record<Vector<Handle<Computation>>> * ret = nullptr;
      sqlite3_stmt * statement;
      std::string queryString = "SELECT COMPUTATIONS from JOB WHERE ID = " + std::to_string(id);
      std::cout << "GetComputations: " << queryString << std::endl;
      if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
           int res = sqlite3_step(statement);
           if (res == SQLITE_ROW) {
               int numBytes = sqlite3_column_bytes(statement, 0);
               void * buffer = (void *) malloc (numBytes);
               memcpy(buffer, sqlite3_column_blob(statement, 0), numBytes);
               ret = (Record<Vector<Handle<Computation>>> *) buffer;
           } 
      } else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;

      }  
      sqlite3_finalize(statement);
      return ret;
}

Handle<Computation> SelfLearningDB :: getComputation(std::string jobName, std::string computationName) {
     Handle<Computation> ret = nullptr;
     Record<Vector<Handle<Computation>>> * recordPtr = nullptr;
     sqlite3_stmt * statement;
     void * buffer = nullptr;
     std::string queryString = "SELECT COMPUTATIONS from JOB WHERE NAME = " + quoteStr(jobName);
     std::cout << "GetComputations: " << queryString << std::endl;
      if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
           int res = sqlite3_step(statement);
           if (res == SQLITE_ROW) {
               int numBytes = sqlite3_column_bytes(statement, 0);
               buffer = (void *) malloc (numBytes);
               memcpy(buffer, sqlite3_column_blob(statement, 0), numBytes);
               recordPtr = (Record<Vector<Handle<Computation>>> *) buffer;
               Handle<Vector<Handle<Computation>>> retrievedComputations = recordPtr->getRootObject();
               int numComputations = retrievedComputations->size();
               for (int i = 0; i < numComputations; i++) {
                   Handle<Computation> curComp = (*retrievedComputations)[i];
                   if(curComp->getComputationName() == computationName) {
                       ret = deepCopyToCurrentAllocationBlock(curComp);
                       break;
                   } 
               }
           }
      } else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;

      }
      sqlite3_finalize(statement);
      if (buffer != nullptr) {
           free(buffer);
      }
      return ret;

}


bool SelfLearningDB :: createJobInstance (long jobId, std::string jobIdForInstance, long& id) {

     id = jobInstanceId;
     jobInstanceId ++;
     string cmdString = "INSERT INTO JOB_INSTANCE "
                " (ID, JOB_ID, JOB_INSTANCE_ID, STATUS, SUBMIT_TIME, FINISH_TIME) "
                "VALUES(" + std::to_string(id) + ","
                          + std::to_string(jobId) + ","
                          + quoteStr(jobIdForInstance) + ","
                          + "'Running',"
                          + "strftime('%s', 'now', 'localtime'), null);";
      std::cout << "CreateJobInstance: " << cmdString << std::endl;
      return execDB(cmdString);
}

bool SelfLearningDB :: updateJobInstanceForCompletion (long id, std::string status) {

      std::string cmdString = std::string("UPDATE JOB_INSTANCE set STATUS = ") + quoteStr(status) +
                              std::string(", FINISH_TIME = strftime('%s', 'now', 'localtime') where ID = ") +
                              std::to_string(id);
      std::cout << "UpdateJobInstanceForCompletion: " << cmdString << std::endl;
      return execDB(cmdString);

}

bool SelfLearningDB :: createJobStage (long jobInstanceId, int jobStageId, 
                std::string stageType, std::string status,
                std::string sourceType, std::string sinkType, std::string probeType,
                Handle<Vector<String>> buildTheseTupleSets, int numPartitions, 
                std::string targetComputationSpecifier, 
                Handle<Computation> aggregationComputation,
                long& id) {

     id = jobInstanceStageId;
     jobInstanceStageId ++;
     sqlite3_stmt * statement;
     string cmdString = "INSERT INTO JOB_STAGE "
                " (ID, JOB_INSTANCE_ID, STAGE_ID, STAGE_TYPE, STATUS, "
                "SOURCE_TYPE, SINK_TYPE, PROBE_TYPE, TUPLESET_SPECIFIERS, "
                "NUM_HASH_KEYS, NUM_PARTITIONS, TARGET_COMPUTATION_SPECIFIER, "
                "AGG_COMPUTATION, START_TIME, FINISH_TIME) "
                "VALUES(" + std::to_string(id) + ","
                          + std::to_string(jobInstanceId) + ","
                          + std::to_string(jobStageId) + ","
                          + quoteStr(stageType) + ","
                          + quoteStr(status) + ","
                          + quoteStr(sourceType) + ","
                          + quoteStr(sinkType) + ","
                          + quoteStr(probeType) + ","
                          + "?,"
                          + "0,"
                          + std::to_string(numPartitions) + ","
                          + quoteStr(targetComputationSpecifier) + ","
                          + "?,"
                          + "strftime('%s', 'now', 'localtime'), null);";
      std::cout << "CreateJobStage: " << cmdString << std::endl;
      char buffer[1];
      buffer[0]=0;
      if (sqlite3_prepare_v2(selfLearningDBHandler, cmdString.c_str(),
             -1, &statement, NULL) == SQLITE_OK) {
                //to get the bytes
                if (buildTheseTupleSets == nullptr) {
                   sqlite3_bind_blob(statement, 1, buffer, 1, SQLITE_STATIC);
                } else {
                   Record<Vector<String>> * record =
                     getRecord<Vector<String>>(buildTheseTupleSets);
                   sqlite3_bind_blob(statement, 1, record, record->numBytes(), SQLITE_STATIC);
                }
                if (aggregationComputation == nullptr) {
                   sqlite3_bind_blob(statement, 2, buffer, 1, SQLITE_STATIC);
                } else {
                   Record<Computation> * record1 =
                     getRecord<Computation>(aggregationComputation);
                   sqlite3_bind_blob(statement, 2, record1, record1->numBytes(), SQLITE_STATIC);
                }
                sqlite3_step(statement);
                sqlite3_finalize(statement);
                return true;
      } else {
                std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
                sqlite3_finalize(statement);
                return false;
      }

}


bool SelfLearningDB :: updateJobStageForCompletion (long id, std::string status) {

      std::string cmdString = std::string("UPDATE JOB_STAGE set STATUS = ") + quoteStr(status) +
                              std::string(", FINISH_TIME = strftime('%s', 'now', 'localtime') where ID = ") +
                              std::to_string(id);
      std::cout << "UpdateJobStageForCompletion: " << cmdString << std::endl;
      return execDB(cmdString);

}

bool SelfLearningDB :: updateJobStageForKeyDistribution (long id, int numHashKeys) {

      std::string cmdString = std::string("UPDATE JOB_STAGE set NUM_HASH_KEYS = ") + std::to_string(numHashKeys) +
                              std::string(" where ID = ") +
                              std::to_string(id);
      std::cout << "UpdateJobStageForKeyDistribution: " << cmdString << std::endl;
      return execDB(cmdString);

}


bool SelfLearningDB :: createDataJobStageMapping (long dataId, long jobInstanceStageId, int indexInInputs, std::string type, long& id) {

     id = dataToJobStageMappingId;
     dataToJobStageMappingId ++;
     string cmdString = "INSERT INTO DATA_JOB_STAGE "
                " (ID, DATA_ID, JOB_STAGE_ID, INDEX_IN_INPUTS, DATA_TYPE) "
                "VALUES(" + std::to_string(id) + ","
                          + std::to_string(dataId) + ","
                          + std::to_string(jobInstanceStageId) + ","
                          + std::to_string(indexInInputs) + ","
                          + quoteStr(type)+");";
      std::cout << "CreateJobStageMapping: " << cmdString << std::endl;
      return execDB(cmdString);
     

}

bool SelfLearningDB :: createLambda (long jobId, 
                                     std::string lambdaType,
                                     std::string lambdaIdentifier,
                                     std::string computationName, 
                                     std::string lambdaName, 
                                     int lambdaInputIndexInComputationInputs, 
                                     std::string lambdaInputClass,
                                     std::string lambdaOutputClass, 
                                     long& id) {

     id = lambdaId;
     lambdaId ++;
     sqlite3_initialize();
     string cmdString = "INSERT INTO LAMBDA "
                 " (ID, JOB_ID, LAMBDA_TYPE, LAMBDA_IDENTIFIER, "
                 "COMPUTATION_NAME, LAMBDA_NAME, "
                 "LAMBDA_INPUT_INDEX_IN_COMPUTATION_INPUTS, LAMBDA_INPUT_CLASS,"
                 " LAMBDA_OUTPUT_CLASS)"
                 " VALUES(" + std::to_string(id) + ","
                            + std::to_string(jobId) + ","
                            + quoteStr(lambdaType) + ","
                            + quoteStr(lambdaIdentifier) + ","
                            + quoteStr(computationName) + ","
                            + quoteStr(lambdaName) + ","
                            + std::to_string(lambdaInputIndexInComputationInputs) + ","
                            + quoteStr(lambdaInputClass) + ","
                            + quoteStr(lambdaOutputClass) + ");";
     std::cout << "CreateLambda: " << cmdString << std::endl;
     bool ret = execDB(cmdString);
     sqlite3_shutdown();
     return ret;
}

bool SelfLearningDB :: getInfoForLoadJob (std::string loadJobName, double & average, double & variance) {

    sqlite3_stmt * statement;
    std::vector<double> sizes;
    std::string queryString = "SELECT SIZE from DATA WHERE CREATED_JOBID = " + quoteStr(loadJobName);
    double totalSize = 0;
    int count = 0;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
          while (1) {
              int res = sqlite3_step(statement);
              if (res == SQLITE_ROW) {
                  size_t size = size_t(sqlite3_column_int64(statement, 0));
                  totalSize +=(double)(size)/(double)(1000000);
                  count ++;
                  sizes.push_back(size);
              } else if (res == SQLITE_DONE) {
                  std::cout << "the end." << std::endl;
                  break;
              } else {
                  std::cout << "Query failed" << std::endl;
                  sqlite3_finalize(statement);
                  return false;
              }
         }
    } else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
           sqlite3_finalize(statement);
           return false;
    }
    sqlite3_finalize(statement);
    average = 0.0;
    if (count > 0) {
       average = (double)totalSize / (double)(count);
    }
    variance = 0;
    for (int i = 0; i < count; i++) {
        variance += ((double)sizes[i] - average) * ((double)sizes[i] - average);
    }
    return true;
}

bool SelfLearningDB :: getAllLambdaCandidates(std::string loadJobName, std::string dataType, std::vector<Handle<LambdaIdentifier>> & lambdaCandidates, RLState & retState) {

    //to get all lambdas that is applicable to the data type
    sqlite3_stmt * statement;
    std::map<std::string, std::vector<LambdaContext>> lambdas;
    std::string queryString = "SELECT ID, JOB_ID, COMPUTATION_NAME, LAMBDA_NAME, LAMBDA_TYPE, LAMBDA_IDENTIFIER, LAMBDA_INPUT_INDEX_IN_COMPUTATION_INPUTS from LAMBDA WHERE LAMBDA_INPUT_CLASS = " + quoteStr(dataType);
    std::cout << "Get lambda context: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        while (1) {
            int res = sqlite3_step(statement);
            if (res == SQLITE_ROW) {
                long lambdaId = sqlite3_column_int(statement, 0);
                long jobId = sqlite3_column_int(statement, 1);
                std::string computationName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
                std::string lambdaName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
                std::string lambdaType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
                std::string lambdaIdentifier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
                int lambdaIndexInInputs = sqlite3_column_int(statement, 6);
                LambdaContext curLambda(lambdaId, jobId, computationName, lambdaName, lambdaIndexInInputs);
                curLambda.identifier = lambdaIdentifier;
                curLambda.typeName = dataType;
                if (lambdaType == "native_lambda") {
                   curLambda.identifier = computationName + "||" + lambdaName;
                }
                lambdas[curLambda.typeName+"||"+curLambda.identifier].push_back(curLambda);
            } else if (res == SQLITE_DONE) {
                break;
            } else {
                std::cout << "Query failed" << std::endl;
                sqlite3_finalize(statement);
                return false;
            }
        }
    } else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
           sqlite3_finalize(statement);
           return false;
    }
    sqlite3_finalize(statement);

    std::cout << "We've identified " << lambdas.size() << " lambdas for partitioning with " << dataType 
              << " for " << loadJobName << std::endl;    

    std::map<int, LambdaContext> candidates;

    int totalCount = 0;

    //for each lambda, we check its data distance, frequency and selectivity
    for (auto a : lambdas) {
        double distance;
        if (a.second[0].typeName == "native_lambda") {
            distance = 1;
        } else if (a.second[0].typeName == "att_access") {
            distance = 0.0;
        } else {
            distance = 0.5;
        }
        double selectivity = 0;
        double numHashKeys = DBL_MAX;
        int count=0;
        for (int i = 0; i < a.second.size(); i++) {
           int freq = 0;
           std::vector<JobStageContext> relatedStages;
           LambdaContext curLambda = a.second[i];
           //select dataId, jobStage that are:
           // --- in the job processing the data that is loaded by the load job, and has the specified job id;
           // --- has target computation that has the specified name
           //by joining data, data_job_stage, job_stage, job_instance
           std::string queryString = "SELECT JOB_STAGE2.JOB_INSTANCE_ID, JOB_STAGE2.ID, JOB_INSTANCE2.JOB_ID, JOB_STAGE2.SINK_TYPE, DATA_JOB_STAGE1.DATA_ID, DATA1.SIZE, DATA_JOB_STAGE2.JOB_STAGE_ID, DATA2.SIZE "
                         "FROM DATA AS DATA1 JOIN DATA_JOB_STAGE AS DATA_JOB_STAGE1 ON (DATA1.ID=DATA_JOB_STAGE1.DATA_ID AND DATA1.CREATED_JOBID="+quoteStr(loadJobName) + ") "
                                            "JOIN JOB_STAGE AS JOB_STAGE1 ON DATA_JOB_STAGE1.JOB_STAGE_ID = JOB_STAGE1.ID "
                                            "JOIN JOB_STAGE AS JOB_STAGE2 ON (JOB_STAGE2.JOB_INSTANCE_ID = JOB_STAGE1.JOB_INSTANCE_ID AND JOB_STAGE2.TARGET_COMPUTATION_SPECIFIER=" + quoteStr(curLambda.computationName) + ") "
                                            "JOIN JOB_INSTANCE AS JOB_INSTANCE2 ON JOB_STAGE2.JOB_INSTANCE_ID = JOB_INSTANCE2.ID "
                                            "JOIN DATA_JOB_STAGE AS DATA_JOB_STAGE2 ON (DATA_JOB_STAGE2.JOB_STAGE_ID = JOB_STAGE2.ID AND DATA_JOB_STAGE2.DATA_TYPE='Sink') "
                                            "JOIN DATA AS DATA2 ON DATA_JOB_STAGE2.DATA_ID = DATA2.ID";


            std::cout << "Get <JOB_INSTANCEID, JOB_STAGE_ID, JOB_ID, SINK_TYPE, DATA_ID, INPUT_SIZE, STAGE_ID, OUTPUT_SIZE>: " << queryString << std::endl;
             // get the historical runs
             // and for each run, we should find out
            if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
               while (1) {
                 int res = sqlite3_step(statement);
                 if (res == SQLITE_ROW) {
                     long jobInstanceId = sqlite3_column_int(statement, 0);
                     long jobStageId = sqlite3_column_int(statement, 1);
                     long jobId = sqlite3_column_int(statement, 2);
                     std::string sinkType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
                     if (sinkType == "PartionedHashSet") {
                        continue;
                     }
                     long dataId = sqlite3_column_int(statement, 4);
                     size_t inputSize = sqlite3_column_int64(statement, 5);
                     int stageId = sqlite3_column_int(statement, 6);
                     size_t outputSize = sqlite3_column_int64(statement, 7);
                     JobStageContext curStage (jobInstanceId, jobStageId, dataId, inputSize, stageId, outputSize);
                     relatedStages.push_back(curStage);
                     freq++;
                 } else if (res == SQLITE_DONE) {
                     break;
                 } else {
                     std::cout << "Query failed" << std::endl;
                     sqlite3_finalize(statement);
                     return false;
                 }
             }
        } else {
             std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
             sqlite3_finalize(statement);
             return false;
        }
        sqlite3_finalize(statement);

        // to set the selectivity
        for (int j = 0; j < freq; j++) {
             JobStageContext curJobStageContext = relatedStages[j];
             double curSelectivity = (double)curJobStageContext.sinkSize / (double)curJobStageContext.sourceSize / (double)(1000000);
             if (curSelectivity > selectivity){
                  selectivity = curSelectivity;
             }
        }

        // to set the key distribution
        for (int j = 0; j < relatedStages.size(); j++) {
             long jobInstanceId = relatedStages[j].jobInstanceId;
             int jobStageId = relatedStages[j].stageId;
             std::string queryString = "SELECT NUM_HASH_KEYS from JOB_STAGE WHERE JOB_INSTANCE_ID = " + std::to_string(jobInstanceId) + " AND ID = " + std::to_string(jobStageId);
             std::cout << "Get NUM_HASH_KEYS: " << queryString << std::endl;
             if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement, NULL) == SQLITE_OK) {
                 int res = sqlite3_step(statement);
                 if (res == SQLITE_ROW) {
                     double  curNumHashKeys = (double)sqlite3_column_int(statement, 0)/(double)(100000);
                     std::cout << "curNumHashKeys = " << curNumHashKeys << std::endl;
                     if (curNumHashKeys < numHashKeys) {
                         numHashKeys = curNumHashKeys;
                     }
                 }

             }
        }
        count = count + freq;
        totalCount = totalCount + freq;
     }
     if (count > 0) {
         LambdaContext curCandidate;
         curCandidate.lambdaId = a.second[0].lambdaId;
         curCandidate.jobId = a.second[0].jobId;
         curCandidate.computationName = a.second[0].computationName;
         curCandidate.lambdaName = a.second[0].lambdaName;
         curCandidate.dataDistance = distance;
         curCandidate.selectivity = selectivity;
         curCandidate.avgNumHashKeys = numHashKeys;
         curCandidate.count = count;
         candidates[curCandidate.lambdaId] = curCandidate;
     }
   }

   //ranking candidates
    int numCandidates = 0;    
    for (auto a : candidates) {
        double distance = a.second.dataDistance;
        double selectivity = a.second.selectivity;
        a.second.frequency = (double)(a.second.count)/(double)(totalCount);
        std::cout << "count = " << a.second.count << ", totalCount = " << totalCount
               << ", frequency = " << a.second.frequency << std::endl;
        double frequency = a.second.frequency;
        std::cout << "frequency is " << frequency << std::endl;
        double numKeys = a.second.avgNumHashKeys;
        if (numCandidates < NUM_CANDIDATE_LAMBDA) {
            //insert this candidate
            retState.candidateLambdaIndex[numCandidates] = a.second.lambdaId;
            std::cout << "retState.candidateLambdaIndex[" << numCandidates <<"]=" << a.second.lambdaId << std::endl;
            retState.dataDistance[numCandidates] = distance;
            retState.frequency[numCandidates] = frequency;
            std::cout << "retState.frequency[" << numCandidates << "]=" << retState.frequency[numCandidates] << std::endl;
            retState.selectivity[numCandidates] = selectivity;
            retState.avgNumHashKeys[numCandidates] = numKeys;
            if (frequency < retState.leastFrequency) {
                retState.indexWithLeastFrequency = numCandidates;
            }
            numCandidates++;
            retState.numCandidates ++;
        } else {
            if (frequency > retState.leastFrequency) {
                retState.candidateLambdaIndex[retState.indexWithLeastFrequency] = a.second.lambdaId;
                std::cout << "retState.candidateLambdaIndex[" << retState.indexWithLeastFrequency <<"]=" << a.second.lambdaId << std::endl;
                retState.dataDistance[retState.indexWithLeastFrequency] = distance;
                retState.frequency[retState.indexWithLeastFrequency] = frequency;
                std::cout << "retState.frequency[" << retState.indexWithLeastFrequency << "]=" 
                          << retState.frequency[retState.indexWithLeastFrequency] << std::endl;
                retState.selectivity[retState.indexWithLeastFrequency] = selectivity;
                retState.avgNumHashKeys[retState.indexWithLeastFrequency] = numKeys;
                retState.indexWithLeastFrequency = -1;
                retState.leastFrequency = DBL_MAX;
                for (int i = 0; i < numCandidates; i++) {
                    if (retState.frequency[i] < retState.leastFrequency) {
                        retState.leastFrequency = retState.frequency[i];
                        retState.indexWithLeastFrequency = i;
                    }
                }
            }
        }   
    }

    //remember to customize below //TODO: remove hard coding
    int dataScale = 10;
    int numNodes = 5;
    std::string diskType = "SSD";
    double memSize = (size_t)(60) * (size_t)(1024) * (size_t)(1024) * (size_t)(1024)/1000000;
    std::string networkType = "10Gb";
    int numCPU = 8;
    retState.dataScale = dataScale;
    if (dataType == "tpch::Customer") {
       retState.inputSize = 2.44847 * (double)((double)dataScale/(double)1000);
    } else if (dataType == "tpch::LineItem") {
       retState.inputSize = 77.75727 * (double)((double)dataScale/(double)1000);
    } else if (dataType == "tpch::Nation") {
       retState.inputSize = 0.00002 * (double)((double)dataScale/(double)1000);
    } else if (dataType == "tpch::Order") {
       retState.inputSize = 17.49195 * (double)((double)dataScale/(double)1000);
    } else if (dataType == "tpch::PartSupp") {
       retState.inputSize = 12.0485 * (double)((double)dataScale/(double)1000);
    } else if (dataType == "tpch::Part") {
       retState.inputSize = 2.43336 * (double)((double)dataScale/(double)1000);
    } else if (dataType == "tpch::Region") {
       retState.inputSize = 0.000003 * (double)((double)dataScale/(double)1000);
    } else if (dataType == "tpch::Supplier") {
       retState.inputSize = 0.14176 * (double)((double)dataScale/(double)1000);
    }
    retState.numNodes = (double)(numNodes)/(double)(20);
    retState.numCores = (double)(numCPU)/(double)(16);
    retState.memSize = (double)(memSize)/(double)(100000);
    if (diskType == "SSD") {
      retState.diskSpeed = 1;
    } else {
      retState.diskSpeed = 0.01;
    }

    if (networkType == "10Gb") {
      retState.networkBandwidth = 1;
    } else {
      retState.networkBandwidth = 0.01;
    }

    // to set output
    const UseTemporaryAllocationBlock tempBlock{32 * 1024 * 1024};
    for (size_t i = 0; i < retState.numCandidates; i++) {
        size_t lambdaId  = retState.candidateLambdaIndex[i];
        LambdaContext curLambda = candidates[lambdaId];
        size_t jobId = curLambda.jobId;
        std::string computationName = curLambda.computationName;
        std::string lambdaName = curLambda.lambdaName;
        std::string jobName = this->getJobName(jobId);
        std::cout << "jobName: " << jobName << ", computationName: " << computationName << ", lambdaName: " << lambdaName << std::endl;
        Handle<LambdaIdentifier> curLambdaIdentifier = makeObject<LambdaIdentifier> (jobName, computationName, lambdaName);
        lambdaCandidates.push_back(curLambdaIdentifier);
    }

    return true;
}
bool SelfLearningDB :: getAllLambdaCandidates1(std::string loadJobName, std::string dataType, std::vector<Handle<LambdaIdentifier>> & lambdaCandidates, RLState & state) {
    
    //to get all lambdas that is applicable to the data type
    sqlite3_stmt * statement;
    std::vector<LambdaContext> lambdas;
    int count = 0;
    std::string queryString = "SELECT ID, JOB_ID, COMPUTATION_NAME, LAMBDA_NAME, LAMBDA_INPUT_INDEX_IN_COMPUTATION_INPUTS from LAMBDA WHERE LAMBDA_INPUT_CLASS = " + quoteStr(dataType);
    std::cout << "Get lambda context: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        while (1) {
            int res = sqlite3_step(statement);
            if (res == SQLITE_ROW) {
                long lambdaId = sqlite3_column_int(statement, 0);
                long jobId = sqlite3_column_int(statement, 1);
                std::string computationName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
                std::string lambdaName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
                int lambdaIndexInInputs = sqlite3_column_int(statement, 4);
                LambdaContext curLambda(lambdaId, jobId, computationName, lambdaName, lambdaIndexInInputs);
                lambdas.push_back(curLambda);
                count ++;
            } else if (res == SQLITE_DONE) {
                break;
            } else {
                std::cout << "Query failed" << std::endl;
                sqlite3_finalize(statement);
                return false;
            }
        }
    } else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
           sqlite3_finalize(statement);
           return false;
    }
    sqlite3_finalize(statement);
   


    //for each lambda, we check its data distance, frequency and selectivity
    for (int i = 0; i < count; i++) {
        std::vector<JobStageContext> relatedStages;
        LambdaContext curLambda = lambdas[i];
        //select dataId, jobStage that are:
        // --- in the job processing the data that is loaded by the load job, and has the specified job id;
        // --- has target computation that has the specified name
        //by joining data, data_job_stage, job_stage, job_instance

        int freq = 0;
        std::string queryString = "SELECT JOB_STAGE2.JOB_INSTANCE_ID, JOB_STAGE2.ID, JOB_INSTANCE2.JOB_ID, JOB_STAGE2.SINK_TYPE, DATA_JOB_STAGE1.DATA_ID, DATA1.SIZE, DATA_JOB_STAGE2.JOB_STAGE_ID, DATA2.SIZE "
                         "FROM DATA AS DATA1 JOIN DATA_JOB_STAGE AS DATA_JOB_STAGE1 ON (DATA1.ID=DATA_JOB_STAGE1.DATA_ID AND DATA1.CREATED_JOBID="+quoteStr(loadJobName) + ") "
                                            "JOIN JOB_STAGE AS JOB_STAGE1 ON DATA_JOB_STAGE1.JOB_STAGE_ID = JOB_STAGE1.ID "
                                            "JOIN JOB_STAGE AS JOB_STAGE2 ON (JOB_STAGE2.JOB_INSTANCE_ID = JOB_STAGE1.JOB_INSTANCE_ID AND JOB_STAGE2.TARGET_COMPUTATION_SPECIFIER=" + quoteStr(curLambda.computationName) + ") "
                                            "JOIN JOB_INSTANCE AS JOB_INSTANCE2 ON JOB_STAGE2.JOB_INSTANCE_ID = JOB_INSTANCE2.ID "
                                            "JOIN DATA_JOB_STAGE AS DATA_JOB_STAGE2 ON (DATA_JOB_STAGE2.JOB_STAGE_ID = JOB_STAGE2.ID AND DATA_JOB_STAGE2.DATA_TYPE='Sink') "
                                            "JOIN DATA AS DATA2 ON DATA_JOB_STAGE2.DATA_ID = DATA2.ID";
                                            
  
        std::cout << "Get <JOB_INSTANCEID, JOB_STAGE_ID, JOB_ID, SINK_TYPE, DATA_ID, INPUT_SIZE, STAGE_ID, OUTPUT_SIZE>: " << queryString << std::endl;
        std::set<long> joinConsumers;
        std::set<long> aggConsumers;
        if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
             while (1) {
                 int res = sqlite3_step(statement);
                 if (res == SQLITE_ROW) {
                     long jobInstanceId = sqlite3_column_int(statement, 0);
                     long jobStageId = sqlite3_column_int(statement, 1);
                     long jobId = sqlite3_column_int(statement, 2);
                     std::string sinkType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
                     if (sinkType == "Repartition") {
                          joinConsumers.insert(jobId);
                     } else if (sinkType == "Shuffle") {
                          aggConsumers.insert(jobId);
                     }
                     long dataId = sqlite3_column_int(statement, 4);
                     size_t inputSize = sqlite3_column_int64(statement, 5);
                     int stageId = sqlite3_column_int(statement, 6);
                     size_t outputSize = sqlite3_column_int64(statement, 7);
                     JobStageContext curStage (jobInstanceId, jobStageId, dataId, inputSize, stageId, outputSize);
                     relatedStages.push_back(curStage);
                     freq++;
                 } else if (res == SQLITE_DONE) {
                     break;
                 } else {
                     std::cout << "Query failed" << std::endl;
                     sqlite3_finalize(statement);
                     return false;
                 }
             } 
        } else {
             std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
             sqlite3_finalize(statement);
             return false;
        }
        sqlite3_finalize(statement);
        //to set join distance: the more different join operators, the smaller join distance
        if(joinConsumers.size() == 0) {
             lambdas[i].joinDistance = -1;
        } else {
             lambdas[i].joinDistance = (double)(10)/(double)(joinConsumers.size());
        }
        std::cout << "lambda[" << i <<"].joinDistance=" << lambdas[i].joinDistance << std::endl;

        //to set agg distance: the more different aggregation operators, the smaller aggregation distance
        if(aggConsumers.size() == 0) {
             lambdas[i].aggDistance = -1;
        } else {
             lambdas[i].aggDistance = (double)(10)/(double)(aggConsumers.size());
        }  
        std::cout << "lambda[" << i <<"].aggDistance=" << lambdas[i].aggDistance << std::endl;

        // to set the count as frequency
        lambdas[i].frequency = freq;
        std::cout << "lambda[" << i <<"].frequency=" << lambdas[i].frequency << std::endl;


        // to set the selectivity
        double sumSelectivity = 0.0;
        for (int j = 0; j < freq; j++) {
             JobStageContext curJobStageContext = relatedStages[j];
             sumSelectivity += (double)curJobStageContext.sinkSize / (double)curJobStageContext.sourceSize;
        }
        if (freq > 0) {
             lambdas[i].selectivity = sumSelectivity/(double)(freq);
        } else {
             lambdas[i].selectivity = 0;
        }
        std::cout << "lambda[" << i <<"].selectivity=" << lambdas[i].selectivity << std::endl;

         
        // to set the key distribution
        int totalNumHashKeys = 0;
        for (int j = 0; j < relatedStages.size(); j++) {
             long jobInstanceId = relatedStages[j].jobInstanceId;
             int jobStageId = relatedStages[j].stageId;
             std::string queryString = "SELECT NUM_HASH_KEYS from JOB_STAGE WHERE JOB_INSTANCE_ID = " + std::to_string(jobInstanceId) + " AND STAGE_ID = " + std::to_string(jobStageId + 1);
             std::cout << "Get NUM_HASH_KEYS: " << queryString << std::endl;
             if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement, NULL) == SQLITE_OK) {
                 int res = sqlite3_step(statement);
                 if (res == SQLITE_ROW) {
                     int numHashKeys = sqlite3_column_int(statement, 0);
                     totalNumHashKeys += numHashKeys;
                 }

             }
        }
        if ((relatedStages.size() > 0) && (totalNumHashKeys > 0)) { 
            lambdas[i].avgNumHashKeys = (double)(totalNumHashKeys)/(double)(relatedStages.size());
        }
        std::cout << "lambda[" << i <<"].avgNumHashKeys=" << lambdas[i].avgNumHashKeys << std::endl;

        // to set the distance;
        int consumerStageId = -1;
        if (relatedStages.size() > 0) { 
            long dataId = relatedStages[0].dataId;
            long jobInstanceId = relatedStages[0].jobInstanceId;
            int targetStageId = relatedStages[0].stageId;
            int inputIndex = curLambda.lambdaIndexInInputs;
            double distance = -1.0;
            int level = -1;
            while (consumerStageId < targetStageId) {
                std::string queryString = "SELECT JOB_STAGE.STAGE_ID, DATA_JOB_STAGE2.DATA_ID, DATA_JOB_STAGE2.INDEX_IN_INPUTS "
                    "from DATA_JOB_STAGE AS DATA_JOB_STAGE1 JOIN JOB_STAGE ON "
                    "(DATA_JOB_STAGE1.JOB_STAGE_ID=JOB_STAGE.ID AND DATA_JOB_STAGE1.DATA_TYPE='Source') "
                    "JOIN DATA_JOB_STAGE AS DATA_JOB_STAGE2 ON "
                    "(DATA_JOB_STAGE2.JOB_STAGE_ID=JOB_STAGE.ID AND DATA_JOB_STAGE2.DATA_TYPE='Sink') "
                    "WHERE DATA_JOB_STAGE1.DATA_ID = " + std::to_string(dataId) +
                    " AND JOB_STAGE.JOB_INSTANCE_ID = " + std::to_string(jobInstanceId) +
                    " AND DATA_JOB_STAGE1.INDEX_IN_INPUTS = " + std::to_string(inputIndex);
                std::cout << "Get Direct Consumer: " << queryString << std::endl;
                if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                      NULL) == SQLITE_OK) {
                     int res = sqlite3_step(statement);
                     if (res == SQLITE_ROW) {
                         consumerStageId = sqlite3_column_int(statement, 0);
                         dataId = sqlite3_column_int(statement, 1);
                         inputIndex = sqlite3_column_int(statement, 2);
                         level++;
                     } else {
                         break;
                     }

                 } else {
                     std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
                     sqlite3_finalize(statement);
                     return false;
                 }
                 sqlite3_finalize(statement);
             }
             if (consumerStageId == targetStageId) {
                 distance = level;
             } 
             lambdas[i].dataDistance = distance;
         }
         std::cout << "lambda[" << i << "].dataDistance=" << curLambda.dataDistance << std::endl;
    }
    //if number of remaining lambda is larger than the limit, we filter the lambda that has smallest selectivity and least frequency
    //sort the vector of lambda based on selectivity*frequency
    double topKMin = -1.0; 
    int minIndex = -1;
    std::vector<LambdaContext> topKLambdas;
    std::cout << "There are " << lambdas.size() << " lambda candidates" << std::endl;
    if (lambdas.size() > NUM_CANDIDATE_LAMBDA) {
        for (int i = 0; i < count; i++) {
            LambdaContext& curLambda = lambdas[i];
            double curScore = curLambda.selectivity * curLambda.frequency;
            if (topKLambdas.size() < NUM_CANDIDATE_LAMBDA) {
                if (topKLambdas.size() == 0) {
                    topKMin = curScore;
                    topKLambdas.push_back(lambdas[i]);
                    minIndex = 0;
                } else {
                    topKLambdas.push_back(lambdas[i]);
                    if (curScore < topKMin) {
                        topKMin = curScore;
                        minIndex = topKLambdas.size() - 1;    
                    }
                }
            } else {
                if (curScore > topKMin) {
                    //to replace the min value
                    topKLambdas[minIndex] = lambdas[i];
                    topKMin = curScore;
                    //to find the new smallest value
                    for (size_t j = 0; j < topKLambdas.size(); j++) {
                        LambdaContext& lambdaToCheck = topKLambdas[j];
                        double scoreToCheck = lambdaToCheck.selectivity * lambdaToCheck.frequency;
                        if (scoreToCheck < topKMin) {
                            topKMin = scoreToCheck;
                            minIndex = j;
                        }
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < lambdas.size(); i++) {
            topKLambdas.push_back(lambdas[i]);
        }
    }

    // to set output
    const UseTemporaryAllocationBlock tempBlock{32 * 1024 * 1024};
    state.numCandidates = count;
    for (size_t i = 0; i < topKLambdas.size(); i++) {
        std::cout << "topKLambda[" << i << "].selectivity=" << topKLambdas[i].selectivity << std::endl;
        std::cout << "topKLambda[" << i << "].frequency=" << topKLambdas[i].frequency << std::endl;
        std::cout << "topKLambda[" << i << "].dataDistance=" << topKLambdas[i].dataDistance << std::endl;
        std::cout << "topKLambda[" << i << "].joinDistance=" << topKLambdas[i].joinDistance << std::endl;
        std::cout << "topKLambda[" << i << "].aggDistance=" << topKLambdas[i].aggDistance << std::endl;
        std::cout << "topKLambda[" << i << "].avgNumHashKeys=" << topKLambdas[i].avgNumHashKeys << std::endl;
        LambdaContext curLambda = topKLambdas[i];
        long jobId = curLambda.jobId;
        std::string computationName = curLambda.computationName;
        std::string lambdaName = curLambda.lambdaName;
        std::string jobName = this->getJobName(jobId);
        std::cout << "jobName: " << jobName << ", computationName: " << computationName << ", lambdaName: " << lambdaName << std::endl;
        Handle<LambdaIdentifier> curLambdaIdentifier = makeObject<LambdaIdentifier> (jobName, computationName, lambdaName);
        lambdaCandidates.push_back(curLambdaIdentifier);
        state.dataDistance[i] = curLambda.dataDistance;
        std::cout << "state.dataDistance[" << i << "]=" << state.dataDistance[i] << std::endl;
        state.frequency[i] = curLambda.frequency;
        std::cout << "state.frequency[" << i << "]=" << state.frequency[i] << std::endl;
        state.selectivity[i] = curLambda.selectivity;
        std::cout << "state.selectivity[" << i << "]=" << state.selectivity[i] << std::endl;
        state.joinDistance[i] = curLambda.joinDistance;
        std::cout << "state.joinDistance[" << i << "]=" << state.joinDistance[i] << std::endl;
        state.aggDistance[i] = curLambda.aggDistance;
        std::cout << "state.aggDistance[" << i << "]=" << state.aggDistance[i] << std::endl;
        state.avgNumHashKeys[i] = curLambda.avgNumHashKeys;
        std::cout << "state.avgNumHashKeys[" << i << "]=" << state.avgNumHashKeys[i] << std::endl;
    }
    return true; 
}


bool SelfLearningDB :: getProducerConsumerInfoForLoadJob(std::string loadJobName, LoadJobSelfLearningInfo& loadJob, std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>, PairKeyHash, PairKeyEqual>& consumers) {

    //to get all historical data generated by jobs having the same loadJobName;
    sqlite3_stmt * statement;
    std::vector<std::pair<long, size_t>> data;
    std::string queryString = "SELECT ID, SIZE from DATA WHERE CREATED_JOBID = " + quoteStr(loadJobName);
      std::cout << "Get data: " << queryString << std::endl;
    size_t totalSize = 0;
    int numInstances = 0;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
          while (1) {
              std::cout << "the " << numInstances << " row:" << std::endl;
              int res = sqlite3_step(statement);
              if (res == SQLITE_ROW) {
                  long dataId = sqlite3_column_int(statement, 0);
                  std::cout << "dataId=" << dataId << std::endl;
                  size_t size = sqlite3_column_int64(statement, 1);
                  std::cout << "size=" << size << std::endl;
                  totalSize += size;
                  numInstances ++;
                  data.push_back(std::pair<long, size_t>(dataId, size));
              } else if (res == SQLITE_DONE) {
                  std::cout << "the end." << std::endl;
                  break;
              } else {
                  std::cout << "Query failed" << std::endl;
                  sqlite3_finalize(statement);
                  return false;
              }
         }
    } else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
           sqlite3_finalize(statement);
           return false;
    }
    sqlite3_finalize(statement);
    loadJob.setNumInstances (numInstances);
    if (numInstances > 0) {
         loadJob.setAvgSize((size_t)((double)totalSize/(double)numInstances));
    } else {
         loadJob.setAvgSize(0);
    }

    //for each data, get its consumer job stages
    std::vector<std::pair<long, int>> consumerJobStages;    
    for (int i = 0; i < data.size(); i++) {
        int indexInInputs = 0;
        queryString = "SELECT JOB_STAGE_ID, INDEX_IN_INPUTS from DATA_JOB_STAGE WHERE DATA_ID = " + std::to_string(data[i].first) 
              + " AND DATA_TYPE = 'Source'";
        std::cout << "Get JOB_STAGE: " << queryString << std::endl;
        long jobStageId = -1;
        if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
              while (1) {
                  int res = sqlite3_step(statement);
                  if (res == SQLITE_ROW) {
                      jobStageId = sqlite3_column_int(statement, 0);
                      indexInInputs = sqlite3_column_int(statement, 1);
                      consumerJobStages.push_back(std::pair<long, int> (jobStageId, indexInInputs));
                  }
                  else if (res == SQLITE_DONE) {
                      break;
                  } else {
                      std::cout << "Query failed" << std::endl;
                      sqlite3_finalize(statement);
                      return false;
                  }
              }
        } else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
           sqlite3_finalize(statement);
           return false;
        }
        sqlite3_finalize(statement);
    }

    for (int i = 0; i < consumerJobStages.size(); i++) {
        long jobStageId = consumerJobStages[i].first;
        int indexInInputs = consumerJobStages[i].second;
        if (jobStageId != -1) {
            long jobId = -1;
            int stageId = -1;
            std::string sinkType = "";
            std::string targetComputation = "";
            //for each consumer job stage, we update the consumers map;
            queryString = "SELECT JOB_ID, STAGE_ID, SINK_TYPE, TARGET_COMPUTATION_SPECIFIER FROM JOB_INSTANCE, JOB_STAGE WHERE JOB_STAGE.ID = " + std::to_string(jobStageId) + " AND JOB_STAGE.JOB_INSTANCE_ID = JOB_INSTANCE.ID";
            std::cout << "Get info from JOB_STAGE: " << queryString << std::endl;
            if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
                int res = sqlite3_step(statement);
                if (res == SQLITE_ROW) {
                    jobId = sqlite3_column_int(statement, 0);
                    stageId = sqlite3_column_int(statement, 1);
                    sinkType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
                    targetComputation = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
                    std::pair<long, long> key(jobId, stageId);
                    if (consumers.count(key) == 0) {
                        consumers[key] 
                            = std::make_shared<JobStageSelfLearningInfo>(sinkType, targetComputation, 
                                jobId, jobStageId, stageId, indexInInputs);
                    }            
                }
            } else {
               std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
               sqlite3_finalize(statement);
               return false;
            }
            sqlite3_finalize(statement);
        }
    }
    

    return true;
}

Handle<LambdaIdentifier> SelfLearningDB :: getLambda(long jobId, std::string computationName, 
       std::string inputType, int indexInInputs) {

    Handle<LambdaIdentifier> ret = nullptr;

    sqlite3_stmt * statement;
    std::string jobName;
    std::string lambdaName;
    std::string lambdaIdentifier;
    long lambdaId;

    //first to get job name
    std::string queryString = "SELECT NAME from JOB WHERE ID = " + std::to_string(jobId);
      std::cout << "Get JobName: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            jobName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
        sqlite3_finalize(statement);
        return ret;
    }
    sqlite3_finalize(statement);

    //then to find the lambda name
    queryString = "SELECT ID, LAMBDA_IDENTIFIER, LAMBDA_NAME from LAMBDA WHERE JOB_ID = " 
         + std::to_string(jobId) + " AND COMPUTATION_NAME = " + quoteStr(computationName)
         + " AND LAMBDA_INPUT_CLASS = " + quoteStr(inputType) 
         + " AND LAMBDA_INPUT_INDEX_IN_COMPUTATION_INPUTS = " + std::to_string(indexInInputs);
    std::cout << "Get LambdaName: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        while (1) {
           int res = sqlite3_step(statement);
           if (res == SQLITE_ROW) {
               lambdaId = sqlite3_column_int(statement, 0);
               lambdaIdentifier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
               lambdaName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
               std::cout << "lambdaIdentifier is " << lambdaIdentifier << std::endl;
               std::cout << "lambdaName is " << lambdaName << std::endl;
               if ((lambdaName == "") || (lambdaIdentifier == "")) continue;
           } else if (res == SQLITE_DONE) {
               break;
           }
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
        sqlite3_finalize(statement);
        return ret;
    }
    sqlite3_finalize(statement);
    if ((lambdaName == "") || (lambdaIdentifier == "")) return nullptr;
    ret = makeObject<LambdaIdentifier>(lambdaId, lambdaIdentifier, inputType, jobName, computationName, lambdaName);
    return ret;
}

Handle<LambdaIdentifier> SelfLearningDB::getLambda(std::string jobName, std::string computationName, std::string lambdaName) {

    Handle<LambdaIdentifier> ret = nullptr;

    sqlite3_stmt * statement;
    long jobId = getJobId(jobName);
    long lambdaId = -1;
    std::string lambdaIdentifier;
    std::string lambdaInputClass;
     

    //then to find the lambda name
    std::string queryString = "SELECT ID, LAMBDA_IDENTIFIER, LAMBDA_INPUT_CLASS from LAMBDA WHERE JOB_ID = "
         + std::to_string(jobId) + " AND COMPUTATION_NAME = " + quoteStr(computationName)
         + " AND LAMBDA_NAME = " + quoteStr(lambdaName);
    std::cout << "Get LambdaName: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        while (1) {
           int res = sqlite3_step(statement);
           if (res == SQLITE_ROW) {
               lambdaId = sqlite3_column_int(statement, 0);
               lambdaIdentifier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
               lambdaInputClass = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
               std::cout << "lambdaIdentifier is " << lambdaIdentifier << std::endl;
               std::cout << "lambdaInputClass is " << lambdaInputClass << std::endl;
           } else if (res == SQLITE_DONE) {
               break;
           }
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
        sqlite3_finalize(statement);
        return ret;
    }
    sqlite3_finalize(statement);

    ret = makeObject<LambdaIdentifier>(lambdaId, lambdaIdentifier, lambdaInputClass, jobName, computationName, lambdaName);
    return ret;

}

Handle<LambdaIdentifier> SelfLearningDB::getPartitionLambda(std::string databaseName, std::string setName) {

    Handle<LambdaIdentifier> ret = nullptr;

    sqlite3_stmt * statement;
    long lambdaId = -1;

    //first to get lambdaId
    std::string queryString = "SELECT LAMBDA_ID from DATA WHERE DATABASE_NAME = " + quoteStr(databaseName) + " AND SET_NAME = " + quoteStr(setName) + " ORDER BY MODIFICATION_TIME DESC";
    std::cout << "Get LAMBDA_ID: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            lambdaId = sqlite3_column_int(statement, 0);
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
        sqlite3_finalize(statement);
        return ret;
    }
    sqlite3_finalize(statement);

    if (lambdaId < 0) {
        return ret;
    }

    long jobId;
    std::string lambdaIdentifier;
    std::string computationName;
    std::string lambdaName;
    std::string lambdaInputClass;

    //then to find the lambda name
    queryString = "SELECT JOB_ID, LAMBDA_IDENTIFIER, COMPUTATION_NAME, LAMBDA_NAME, LAMBDA_INPUT_CLASS from LAMBDA WHERE ID = "
         + std::to_string(lambdaId);
    std::cout << "Get LambdaName: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        while (1) {
           int res = sqlite3_step(statement);
           if (res == SQLITE_ROW) {
               jobId = sqlite3_column_int(statement, 0);
               lambdaIdentifier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
               computationName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
               lambdaName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
               lambdaInputClass = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
               std::cout << "jobId is " << jobId << std::endl;
               std::cout << "lambdaIdentifier is " << lambdaIdentifier << std::endl;
               std::cout << "computationName is " << computationName << std::endl;
               std::cout << "lambdaName is " << lambdaName << std::endl;
               std::cout << "lambdaInputClass is " << lambdaInputClass << std::endl;
           } else if (res == SQLITE_DONE) {
               break;
           }
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
        sqlite3_finalize(statement);
        return ret;
    }
    sqlite3_finalize(statement);
    std::string jobName = getJobName(jobId);
    ret = makeObject<LambdaIdentifier>(lambdaId, lambdaIdentifier, lambdaInputClass, jobName, computationName, lambdaName);
    return ret;

}

size_t SelfLearningDB::getDataSize (std::string loadJobName, long jobInstanceId) {
    sqlite3_stmt * statement;
    std::string queryString = "SELECT DATA.SIZE FROM DATA JOIN DATA_JOB_STAGE ON DATA.ID = DATA_JOB_STAGE.DATA_ID "
                              " JOIN JOB_STAGE ON DATA_JOB_STAGE.JOB_STAGE_ID = JOB_STAGE.ID "
                              " JOIN JOB_INSTANCE ON JOB_STAGE.JOB_INSTANCE_ID = JOB_INSTANCE.ID "
                              " WHERE DATA.CREATED_JOBID = " + quoteStr(loadJobName) +
                              " AND DATA_JOB_STAGE.DATA_TYPE = 'Source' AND JOB_INSTANCE.ID = " +
                              std::to_string(jobInstanceId);

    std::cout << "GET DATA size: " << queryString << std::endl;
    size_t size = 0;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            size = sqlite3_column_int64(statement, 0);
            std::cout << "size is " << size << std::endl;
        } else {
            std::cout << "Query failed" << std::endl;
            sqlite3_finalize(statement);
        }
    }
    return size;
}

bool SelfLearningDB::getReward(std::string loadJobName, double &reward) {
     long lastDataId = this->getLatestDataIdForLoadJob(loadJobName);
     std::cout << "lastDataId for " << loadJobName << " is " << lastDataId << std::endl;
     long lambdaId = this->getLambdaId(lastDataId);
     sqlite3_stmt * statement;
     std::vector<JobInstanceContext> instances;
     //to get all jobs 
     std::string queryString = "SELECT JOB.ID, JOB_INSTANCE.ID, DATA.SIZE, JOB.INITIAL_LATENCY, JOB.INITIAL_JOB_INSTANCE_ID, JOB_INSTANCE.SUBMIT_TIME, JOB_INSTANCE.FINISH_TIME "
                               "FROM DATA JOIN DATA_JOB_STAGE ON DATA.ID = DATA_JOB_STAGE.DATA_ID "
                                         "JOIN JOB_STAGE ON DATA_JOB_STAGE.JOB_STAGE_ID = JOB_STAGE.ID "
                                         "JOIN JOB_INSTANCE ON JOB_STAGE.JOB_INSTANCE_ID = JOB_INSTANCE.ID "
                                         "JOIN JOB ON JOB_INSTANCE.JOB_ID = JOB.ID "
                                         "WHERE DATA.ID = " + std::to_string(lastDataId) + 
                                         " AND DATA_JOB_STAGE.DATA_TYPE = 'Source' AND JOB_INSTANCE.STATUS = 'Succeeded'";
     std::cout << "GET Execution time: " << queryString << std::endl;
     if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        
        while (1) {
            int res = sqlite3_step(statement);
            if (res == SQLITE_ROW) {
                long jobId = sqlite3_column_int(statement, 0);
                long jobInstanceId = sqlite3_column_int(statement, 1);
                size_t size = sqlite3_column_int64(statement, 2);
                std::cout << "size is " << size << std::endl;
                double initialLatency = sqlite3_column_double(statement, 3);
                std::cout << "initial latency is " << initialLatency << std::endl;
                long initialJobInstanceId = sqlite3_column_int(statement, 4);
                long startTime = sqlite3_column_int(statement, 5);
                long endTime = sqlite3_column_int(statement, 6);
                JobInstanceContext curInstance (jobId, jobInstanceId, size, initialLatency, initialJobInstanceId, startTime, endTime);
                instances.push_back(curInstance);
            } else if (res == SQLITE_DONE) {
                break;
            } else {
                std::cout << "Query failed" << std::endl;
                sqlite3_finalize(statement);
                return false;
            }
        }
     
     } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
        sqlite3_finalize(statement);
        return false;
    }
    sqlite3_finalize(statement);
   
    reward = 0.0;
    int numInstances = instances.size();
    for (int i = 0; i < numInstances; i++) {
        JobInstanceContext curInstance = instances[i];
        double curThroughput = 0;
        double curLatency = 0;
        long curJobInstanceId = 0;
        double initialThroughput = 0;
        size_t size = 0;
        if (curInstance.inputSize > 0) {
            curLatency = (double)(curInstance.endTime - curInstance.submitTime);
            curThroughput = ((double) curInstance.inputSize / (double)(1000000)) / curLatency;
            std::cout << "current input size: " << curInstance.inputSize << std::endl;
            std::cout << "start time: " << curInstance.submitTime << std::endl;
            std::cout << "end time: " << curInstance.endTime << std::endl;
            std::cout << "latency: " << curLatency << std::endl;
            std::cout << "current throughput (MB/s): " << curThroughput << std::endl;
        }
        if ((curInstance.initialLatency < 0)&&(curLatency > 0)) {
            std::cout << "update job initial latency for job-" 
                 << curInstance.jobId << " to be " << curLatency << std::endl;
            std::cout << "update job instance id to be "
                 << curInstance.jobInstanceId << std::endl;
            this->updateJobForInitialLatency(curInstance.jobId, curLatency, curInstance.jobInstanceId);
            reward = 1.0;
        } else {
            curJobInstanceId = curInstance.jobInstanceId;
            size = getDataSize (loadJobName, curJobInstanceId);
            std::cout << "initial size is " << size << std::endl;
            initialThroughput = ((double)(size) / (double)(1000000)) / curInstance.initialLatency;
            std::cout << "initial throughput is " << initialThroughput << std::endl;
            std::cout << "current throughput is " << curThroughput << std::endl;
            reward = reward + ((double)(curThroughput)/(double)(initialThroughput));
        }
    }
    if (numInstances > 0) {
       reward = reward / (double) (numInstances);
    }

    if (numInstances == 0) {
       reward = 0; //it has no influence to the performance
    }
 
    return true;
       
}

bool SelfLearningDB::getSinkData (long jobStageId, std::pair<std::string, std::string>& data) {

    //to get the data id    
    sqlite3_stmt * statement;
    long dataId = getSinkDataId (jobStageId);

    //to get the data info
    std::string queryString = "SELECT DATABASE_NAME, SET_NAME FROM DATA WHERE ID = "
        + std::to_string(dataId);
    std::cout << "GET DATA info: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            data.first = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
            data.second = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
        sqlite3_finalize(statement);
        return false;
    }
    sqlite3_finalize(statement);
    return true;
     
}

long SelfLearningDB::getJobId (std::string jobName) {

   sqlite3_stmt * statement;
   long jobId = -1;
   std::string queryString = "SELECT ID FROM JOB WHERE NAME = " + quoteStr(jobName);
   std::cout << "Get JOB_ID: " << queryString << std::endl;
   if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            jobId = sqlite3_column_int(statement, 0);
        }
   } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
   }
   sqlite3_finalize(statement);
   return jobId;

}

long SelfLearningDB::getLambdaId (long dataId) {

   sqlite3_stmt * statement;
   long lambdaId = -1;
   std::string queryString = "SELECT LAMBDA_ID FROM DATA WHERE ID = " +
         std::to_string(dataId);
   std::cout << "Get LAMBDA_ID: " << queryString << std::endl;
   if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            lambdaId = sqlite3_column_int(statement, 0);
        }
   } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
   }
   sqlite3_finalize(statement);
   return lambdaId;

}


long SelfLearningDB::getLambdaId (std::string jobName, std::string computationName, std::string lambdaName) {

    sqlite3_stmt * statement;
    long lambdaId = -1;
    long jobId = getJobId(jobName);
    std::string queryString = "SELECT ID FROM LAMBDA WHERE JOB_ID = " + std::to_string(jobId)
                              + " AND COMPUTATION_NAME = " + quoteStr(computationName)
                              + " AND LAMBDA_NAME = " + quoteStr(lambdaName);
    std::cout << "Get LAMBDA_ID: " << queryString << std::endl;
   if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            lambdaId = sqlite3_column_int(statement, 0);
        }
   } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
   }
   sqlite3_finalize(statement);
   return lambdaId;

}


long SelfLearningDB::getSinkDataId (long jobStageId) {

    sqlite3_stmt * statement;
    long dataId = -1;
    std::string queryString = "SELECT DATA_ID FROM DATA_JOB_STAGE WHERE JOB_STAGE_ID = "
         + std::to_string(jobStageId) + " AND DATA_TYPE = 'Sink'";
    std::cout << "Get DATA_ID: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            dataId = sqlite3_column_int(statement, 0);
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
    }
    sqlite3_finalize(statement);
    return dataId;   

}

std::string SelfLearningDB::getDataType (long dataId) {

    sqlite3_stmt * statement;
    std::string dataType = "";
    std::string queryString = "SELECT CLASS_NAME FROM DATA WHERE ID = "
         + std::to_string(dataId);
    std::cout << "Get CLASS_NAME: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            dataType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
    }
    sqlite3_finalize(statement);
    return dataType;

}

std::string SelfLearningDB::getJobName (long jobId) {

    sqlite3_stmt * statement;
    std::string jobName = "";
    std::string queryString = "SELECT NAME FROM JOB WHERE ID = "
         + std::to_string(jobId);
    std::cout << "Get NAME: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            jobName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
            std::cout << "jobName is " << jobName << std::endl;
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
    }
    sqlite3_finalize(statement);
    std::cout << "jobName is " << jobName << std::endl;
    return jobName;

}


std::string SelfLearningDB::getJobName (std::string job_instance_id) {

    sqlite3_stmt * statement;
    long jobId = -1;
    std::string queryString = "SELECT JOB_ID FROM JOB_INSTANCE WHERE JOB_INSTANCE_ID = "
         + quoteStr(job_instance_id);
    std::cout << "Get JOB_ID: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
        int res = sqlite3_step(statement);
        if (res == SQLITE_ROW) {
            jobId = sqlite3_column_int(statement, 0);
        }
    } else {
        std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
    }
    sqlite3_finalize(statement);
    std::string jobName = getJobName(jobId);
    return jobName;

}

bool SelfLearningDB::getConsumerInfoForData(long dataId,
                  std::unordered_map<std::pair<long, long>, std::shared_ptr<JobStageSelfLearningInfo>,
                  PairKeyHash, PairKeyEqual>& consumers) {

    sqlite3_stmt * statement;
    std::vector<std::pair<long, int>> consumerJobStages;
    // 1. to get id of the first consumer job stage;
    int indexInInputs = 0;
    std::string queryString = "SELECT JOB_STAGE_ID, INDEX_IN_INPUTS from DATA_JOB_STAGE WHERE DATA_ID = " + std::to_string(dataId)
          + " AND DATA_TYPE = 'Source'";
    std::cout << "Get JOB_STAGES: " << queryString << std::endl;
    long jobStageId = -1;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
          while (1) {
              int res = sqlite3_step(statement);
              if (res == SQLITE_ROW) {
                   jobStageId = sqlite3_column_int(statement, 0);
                   indexInInputs = sqlite3_column_int(statement, 1);
                   std::cout << "to push back jobStageId=" << jobStageId 
                             << " and indexInInputs=" << indexInInputs << std::endl;
                   consumerJobStages.push_back(std::pair<long, int>(jobStageId, indexInInputs));
              } else if (res == SQLITE_DONE) {
                   break;
              }
          }
    } else {
           std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
           sqlite3_finalize(statement);
           return false;
    }
    sqlite3_finalize(statement);    

    // 2. update the consumers map
    for (size_t i = 0; i < consumerJobStages.size(); i++) {
           long jobStageId = consumerJobStages[i].first;
           int indexInInputs = consumerJobStages[i].second;
           long jobId = -1;
           int stageId = -1;
           std::string sinkType = "";
           std::string targetComputation = "";
           //for each consumer job stage, we update the consumers map;
           queryString = "SELECT JOB_ID, STAGE_ID, SINK_TYPE, TARGET_COMPUTATION_SPECIFIER FROM JOB_INSTANCE, JOB_STAGE WHERE JOB_STAGE.ID = " + std::to_string(jobStageId) + " AND JOB_STAGE.JOB_INSTANCE_ID = JOB_INSTANCE.ID";
           std::cout << "Get info from JOB_STAGE: " << queryString << std::endl;
           if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
                     NULL) == SQLITE_OK) {
                int res = sqlite3_step(statement);
                if (res == SQLITE_ROW) {
                    jobId = sqlite3_column_int(statement, 0);
                    stageId = sqlite3_column_int(statement, 1);
                    sinkType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
                    targetComputation = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
                    std::pair<long, long> key(jobId, stageId);
                    if (consumers.count(key) == 0) {
                        consumers[key]
                            = std::make_shared<JobStageSelfLearningInfo>(sinkType, targetComputation,
                                jobId, jobStageId, stageId, indexInInputs);
                    }
                }
           } else {
               std::cout << (std::string)(sqlite3_errmsg(selfLearningDBHandler)) << std::endl;
               sqlite3_finalize(statement);
               return false;
           }
           sqlite3_finalize(statement);
    }
    return true;
}

}



#endif
