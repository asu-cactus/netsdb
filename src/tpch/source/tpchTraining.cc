#ifndef TPCH_TRAINING_CC
#define TPCH_TRAINING_CC

#include "CatalogClient.h"

#include <cfloat>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <chrono>
#include <sstream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>
#include <sqlite3.h>

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"
#include "TPCHSchema.h"
#include "Handle.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SelectionComp.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include "QueryOutput.h"
#include "DataTypes.h"
#include "RLState.h"
#include "RLClient.h"


#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>
#include <cstdlib>

using namespace pdb;
using namespace tpch;


#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define BLOCKSIZE (64 * MB)



static sqlite3 * selfLearningDBHandler;


bool openDB() {

    std::string pathToDB = "selfLearningDB/dbFile";
    if (sqlite3_open_v2(pathToDB.c_str(), &selfLearningDBHandler,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL) == SQLITE_OK) {

        sqlite3_exec(selfLearningDBHandler, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
        std::cout << "self learning database open successfully" << std::endl;
        return true;
    } else {
        std::cout << "failure in opening self learning database" << std::endl;
        return false;
    }
    

}

bool execDB(std::string cmdString) {

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

typedef struct LambdaStat {
    int id;
    double distance;
    double frequency;
    double selectivity;
    double numKeys;
} LambdaStat_t;

bool getState(const std::string & dataType, 
              std::map<std::string, int> & querySet, 
              const int & workloadSize, 
              const std::vector<std::string> & workloadSet,
              const int & environmentId,
              RLState& retState) {
    retState.clear();    
    //get all consuming lambdas for this set and all related queries;
    sqlite3_stmt * statement;
    std::map<int, std::vector<LambdaStat_t>> candidates;
    for (size_t i = 0; i < workloadSet.size(); i++) {
        std::string queryName = workloadSet[i];
        int count = querySet[queryName];
        std::cout << "Workload has " << count << queryName << std::endl;
        if (count == 0) {
            continue;
        }
        double ratio = (double)(count)/(double)(workloadSize);
        std::string queryString = "SELECT QUERY.LAMBDA_ID, QUERY.DISTANCE, QUERY.SELECTIVITY, "
                              "QUERY.KEY_DISTRIBUTION, LAMBDA.LAMBDA_TYPE FROM "
                              "QUERY_LAMBDA_STAT AS QUERY JOIN LAMBDA_STAT AS lAMBDA "
                              "ON LAMBDA.ID = QUERY.LAMBDA_ID AND LAMBDA.TYPE_NAME = '" + dataType + "' "
                              "AND QUERY.JOB_NAME = '" + queryName + "' "
                              "AND QUERY.ENVIRONMENT_ID =" + std::to_string(environmentId); 

        std::cout << "to get feature vector: " << queryString << std::endl; 
        if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1,
            &statement, NULL) == SQLITE_OK) {
             int res;
             while ((res = sqlite3_step(statement)) == SQLITE_ROW) {
                 int lambdaId = sqlite3_column_int64(statement, 0);
                int distance = sqlite3_column_int(statement, 1);
                 float selectivity = sqlite3_column_double(statement, 2);
                 float numKeys = sqlite3_column_int64(statement, 3)/100000;
                 std::string lambdaType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
                 if (lambdaType == "native_lambda") {
                     distance = 5;
                 } else if (lambdaType == "attAccess") {
                     distance = 1;
                 } else {
                     distance = 3;
                 }
                 LambdaStat_t curLambda;
                 curLambda.id = lambdaId;
                 curLambda.distance = distance;
                 curLambda.selectivity = selectivity;
                 curLambda.frequency = ratio;
                 curLambda.numKeys = numKeys;
                 candidates[lambdaId].push_back(curLambda);
             }
        } else {
             std::cout << "failure in opening self learning database" << std::endl;
             return false;
        }    

    }
    int numCandidates = 0;    
    for (auto lambda : candidates) {
        std::cout << lambda.first << ":" << lambda.second.size() << std::endl;
        double distance = lambda.second[0].distance;
        double selectivity = 0;
        double frequency = 0;
        double numKeys = DBL_MAX;
        for (int i = 0; i < lambda.second.size(); i++) {
            if (lambda.second[i].selectivity > selectivity) {
                selectivity = lambda.second[i].selectivity;
            } 
            if (lambda.second[i].numKeys < numKeys) {
                numKeys = lambda.second[i].numKeys;
            }
            frequency += lambda.second[i].frequency;
        }
        if (numCandidates < NUM_CANDIDATE_LAMBDA) {
            //insert this candidate
            retState.candidateLambdaIndex[numCandidates] = lambda.second[0].id;
            std::cout << "retState.candidateLambdaIndex[" << numCandidates <<"]=" << lambda.second[0].id << std::endl;
            retState.dataDistance[numCandidates] = distance;
            retState.frequency[numCandidates] = frequency;
            retState.selectivity[numCandidates] = selectivity;
            retState.avgNumHashKeys[numCandidates] = numKeys;
            if (frequency < retState.leastFrequency) {
                retState.indexWithLeastFrequency = numCandidates;
            }
            numCandidates++;
            retState.numCandidates ++;
        } else {
            if (frequency > retState.leastFrequency) {
                retState.candidateLambdaIndex[retState.indexWithLeastFrequency] = lambda.second[0].id;
                std::cout << "retState.candidateLambdaIndex[" << retState.indexWithLeastFrequency <<"]=" << lambda.second[0].id << std::endl;
                retState.dataDistance[retState.indexWithLeastFrequency] = distance;
                retState.frequency[retState.indexWithLeastFrequency] = frequency;
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
    retState.environmentId = environmentId;
    std::string queryString = "SELECT DATA_SCALE, NUM_NODES, INSTANCE_TYPE, "
                  "DISK_TYPE, MEM_SIZE, NETWORK_TYPE, NUM_CPU FROM "
                  "ENVIRONMENT_STAT WHERE ID= " +std::to_string(environmentId);
    std::cout << "to get environment: " << queryString << std::endl;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1,
        &statement, NULL) == SQLITE_OK) {
        int res;
        if ((res = sqlite3_step(statement)) == SQLITE_ROW) {
            int dataScale = sqlite3_column_int(statement, 0);
            int numNodes = sqlite3_column_int(statement, 1);
            std::string instanceType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
            std::string diskType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
            double memSize = sqlite3_column_int64(statement, 4)/1000000;
            std::string networkType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
            int numCPU = sqlite3_column_int(statement, 6);
            retState.dataScale = dataScale;
            if (dataType == "Customer") {
                retState.inputSize = 2.44847 * (double)(dataScale);
            } else if (dataType == "LineItem") {
                retState.inputSize = 77.75727 * (double)(dataScale);
            } else if (dataType == "Nation") {
                retState.inputSize = 0.00002 * (double)(dataScale);
            } else if (dataType == "Orders") {
                retState.inputSize = 17.49195 * (double)(dataScale);
            } else if (dataType == "PartSupp") {
                retState.inputSize = 12.0485 * (double)(dataScale);
            } else if (dataType == "Part") {
                retState.inputSize = 2.43336 * (double)(dataScale);
            } else if (dataType == "Region") {
                retState.inputSize = 0.000003 * (double)(dataScale);
            } else if (dataType == "Supplier") {
                retState.inputSize = 0.14176 * (double)(dataScale);
            }
            retState.numNodes = numNodes;
            retState.numCores = numCPU;
            retState.memSize = memSize;
            if (diskType == "SSD") {
               retState.diskSpeed = 1000;
            } else {
               retState.diskSpeed = 1;
            }

            if (networkType == "10Gb") {
               retState.networkBandwidth = 1000;
            } else {
               retState.networkBandwidth = 1;
            }
        }
    } else {
        std::cout << "failure in opening self learning database" << std::endl;
        return false;
    }
    
    return true;
}

double getReward(const int& customerIndex, 
                const int& lineitemIndex, 
                const int& nationIndex, 
                const int& orderIndex, 
                const int& partIndex, 
                const int& partsuppIndex, 
                const int& regionIndex,
                const int& supplierIndex,
                std::map<std::string, int>& querySet,
                int dataScale,
                int environmentId) {
    sqlite3_stmt * statement;
    std::string queryString = "SELECT ID FROM PARTITION_SCHEME_STAT "
                  "WHERE CUSTOMER_LAMBDA_ID = " +std::to_string(customerIndex) +
                  " AND LINEITEM_LAMBDA_ID = " +std::to_string(lineitemIndex) +
                  " AND NATION_LAMBDA_ID = " +std::to_string(nationIndex) +
                  " AND ORDER_LAMBDA_ID = " +std::to_string(orderIndex) +
                  " AND PART_LAMBDA_ID = " +std::to_string(partIndex) + 
                  " AND PARTSUPP_LAMBDA_ID = " +std::to_string(partsuppIndex) +
                  " AND REGION_LAMBDA_ID = " +std::to_string(regionIndex) +
                  " AND SUPPLIER_LAMBDA_ID = " +std::to_string(supplierIndex);
    std::cout << "to get partitionId: " << queryString << std::endl;
    int partitionId = -1;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1,
        &statement, NULL) == SQLITE_OK) {
        int res;
        if ((res = sqlite3_step(statement)) == SQLITE_ROW) {
            partitionId = sqlite3_column_int64(statement, 0);
        } else {
            return -1;
        }
    } else {
        std::cout << "failure in opening self learning database" << std::endl;
        return -1;
    }

    for (auto query : querySet) {
        queryString = "SELECT LATENCY FROM RUN_STAT WHERE JOB_NAME = '" +
                      query.first + "' AND PARTITION_SCHEME_ID = " +
                      std::to_string(partitionId) + " AND ENVIRONMENT_ID = " +
                      std::to_string(environmentId);
        if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1,
            &statement, NULL) == SQLITE_OK) {
            int res;
            if ((res = sqlite3_step(statement)) == SQLITE_ROW) {
                double latency = sqlite3_column_double(statement, 0);
                return (double)(dataScale)*1000/latency;
        } else {
            return -1;
        }
    } else {
        std::cout << "failure in opening self learning database" << std::endl;
        return -1;
    }
    }
    return -1;
}




int main(int argc, char* argv[]) {

    int numIterations = 2;
    if (argc == 2) {
        numIterations = atoi(argv[1]);
    }

    if (openDB() == false) {
        return false;
    }
    int maxWorkloadSize = 100;
    int numEnvironments = 1;


    std::vector<std::string> workloadSet = {"TPCHQuery01", "TPCHQuery02", "TPCHQuery03"};
    srand(time(NULL));
    double lastReward = 0;
    PDBLoggerPtr logger = std::make_shared<PDBLogger>("training.log");
    std::shared_ptr<RLClient> rlClient = std::make_shared<RLClient> (8333, "localhost", logger); 

    
    for (int iter = 0; iter < numIterations; iter++) {

        //1. sample a workload size 
        int curWorkloadSize = maxWorkloadSize * ((double)(rand())/(double)(RAND_MAX));
        std::cout << "sampled workload size is " << curWorkloadSize << std::endl;
        
        //2. sample an environment
        int curEnvironment;
        if (numEnvironments == 1) {
            curEnvironment = 0;
        } else {
            curEnvironment = (numEnvironments * ((int)((double)(rand())/(double)(RAND_MAX))))%numEnvironments;
        }
        //3. sample a workload set
        std::map<std::string, int> querySet;
        for (int i = 0; i < curWorkloadSize; i++) {
            int curQueryId = (int)(((double)workloadSet.size()) * ((double)((double)(rand())/(double)(RAND_MAX)))) % workloadSet.size();
            std::string curWorkload = workloadSet[curQueryId];
            std::cout << i << ":" << curWorkload << "; ";
            if (querySet.count(curWorkload) == 0) {
                querySet[curWorkload] = 1;
            } else {
                querySet[curWorkload] = querySet[curWorkload]+1;
            }
        }
        std::cout << std::endl;

        //3. in a loop for each set:
        //-- send last reward and state vector to the RL server
        //-- get the best lambda as action
        RLState customerState, lineitemState, nationState, orderState, partState, partsuppState, regionState, supplierState;
        std::string errMsg;

        getState("Customer", querySet, curWorkloadSize, workloadSet, curEnvironment, customerState);
        int customerLambdaIndexId = rlClient->getBestLambdaIndex(customerState, lastReward, errMsg);
        customerState.print();

        getState("LineItem", querySet, curWorkloadSize, workloadSet, curEnvironment, lineitemState);
        int lineitemLambdaIndexId = rlClient->getBestLambdaIndex(lineitemState, lastReward, errMsg);
        lineitemState.print();

        getState("Nation", querySet, curWorkloadSize, workloadSet, curEnvironment, nationState);
        int nationLambdaIndexId = rlClient->getBestLambdaIndex(nationState, lastReward, errMsg);
        nationState.print();

        getState("Order", querySet, curWorkloadSize, workloadSet, curEnvironment, orderState);
        int orderLambdaIndexId = rlClient->getBestLambdaIndex(orderState, lastReward, errMsg);
        orderState.print();

        getState("Part", querySet, curWorkloadSize, workloadSet, curEnvironment, partState);
        int partLambdaIndexId = rlClient->getBestLambdaIndex(partState, lastReward, errMsg);
        partState.print();

        getState("PartSupp", querySet, curWorkloadSize, workloadSet, curEnvironment, partsuppState);
        int partsuppLambdaIndexId = rlClient->getBestLambdaIndex(partsuppState, lastReward, errMsg);
        partsuppState.print();

        getState("Region", querySet, curWorkloadSize, workloadSet, curEnvironment, regionState);
        int regionLambdaIndexId = rlClient->getBestLambdaIndex(regionState, lastReward, errMsg);
        regionState.print();

        getState("Supplier", querySet, curWorkloadSize, workloadSet, curEnvironment, supplierState);
        int supplierLambdaIndexId = rlClient->getBestLambdaIndex(supplierState, lastReward, errMsg);
        supplierState.print();


        //4. compute reward based on partition Ids
        double reward = getReward(customerState.getLambdaId(customerLambdaIndexId),
                                  lineitemState.getLambdaId(lineitemLambdaIndexId),
                                  nationState.getLambdaId(nationLambdaIndexId),
                                  orderState.getLambdaId(orderLambdaIndexId),
                                  partState.getLambdaId(partLambdaIndexId),
                                  partsuppState.getLambdaId(partsuppLambdaIndexId),
                                  regionState.getLambdaId(regionLambdaIndexId),
                                  supplierState.getLambdaId(supplierLambdaIndexId),
                                  querySet,
                                  supplierState.dataScale,
                                  supplierState.environmentId);      
        std::cout << "reward:" << reward << std::endl;
        //5. update reward
        if (reward > 0) {
            lastReward = reward;
        }
    }


}
#endif
