#ifndef TPCH_PREPARE_TRAINING_CC
#define TPCH_PREPARE_TRAINING_CC

#include "CatalogClient.h"

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

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

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

typedef struct LambdaStruct {

    long lambdaId;
    std::string jobName;
    std::string computationName;
    std::string lambdaName;
    std::string typeName;
    std::string lambdaType;
    std::string lambdaIdentifier;

} LambdaStruct_t;

int main(int argc, char* argv[]) {

    
    //prerequisite: run all queries once

    //generate four tables called LambdaStatistics, PartitionStatistics, ENvironmentStatistics and QueryLambdaStatistics respectively based on those initial runs

    // LambdaStatistics
    // --id, the id of partition lambda
    // --jobName, the name of the job
    // --computationName, the name of the computation
    // --lambdaName, the name of the lambda
    // --typeName, the name of the input type
    
    // PartitionSchemeStatistics
    // --id, the id of partition scheme 
    // --customerLambda, lambda id for partitioning the customer table
    // --lineitemLambda, lambda id for partitioning the lineitem table
    // --orderLambda, lambda id for partitioning the order table
    // --partLambda, lambda id for partitioning the part table
    // --supplierLambda, lambda id for partitioning the supplier table
    // --partsuppLambda, lambda id for partitioning the partsupp table
    // --regionLambda, lambda id for partitioning the region table
    // --nationLambda, lambda id for partitioning the nation table

    // EnvironmentStatisitcs
    // --id, the id of environment
    // --dataScale
    // --numNodes
    // --instanceType
    // --diskType
    // --memSize
    // --networkType
    // --numCPU

    // --id, the id of the query lambda pair
    // --jobName, the name of the query
    // --lambdaId, the id of the lambda
    // --environmentId, the id of the environment
    // --distance
    // --selectivity
    // --keyDistribution

    // to create tables
    if (openDB() == false) {
        return false;
    }

    


    execDB ("DROP TABLE IF EXISTS LAMBDA_STAT;");
    execDB ("CREATE TABLE IF NOT EXISTS LAMBDA_STAT (ID BIGINT PRIMARY KEY, "
            "LAMBDA_TYPE VARCHAR(32), LAMBDA_IDENTIFIER VARCHAR(128), "
            "JOB_NAME VARCHAR(128), COMPUTATION_NAME VARCHAR(128), "
            "LAMBDA_NAME VARCHAR(128), TYPE_NAME VARCHAR(128)) WITHOUT ROWID;");

    execDB ("DROP TABLE IF EXISTS PARTITION_LAMBDA_STAT;");
    execDB ("CREATE TABLE IF NOT EXISTS PARTITION_LAMBDA_STAT (ID BIGINT PRIMARY KEY, "
            "JOB_NAME VARCHAR(128), COMPUTATION_NAME VARCHAR(128), "
            "LAMBDA_NAME VARCHAR(128), TYPE_NAME VARCHAR(128)) WITHOUT ROWID;");

    execDB ("DROP TABLE IF EXISTS PARTITION_SCHEME_STAT;");
    execDB ("CREATE TABLE IF NOT EXISTS PARTITION_SCHEME_STAT (ID BIGINT PRIMARY KEY, "
            "CUSTOMER_LAMBDA_ID BIGINT, LINEITEM_LAMBDA_ID BIGINT, ORDER_LAMBDA_ID BIGINT, "
            "PART_LAMBDA_ID BIGINT, SUPPLIER_LAMBDA_ID BIGINT, PARTSUPP_LAMBDA_ID BIGINT, "
            "REGION_LAMBDA_ID BIGINT, NATION_LAMBDA_ID BIGINT) WITHOUT ROWID;" );

    execDB ("DROP TABLE IF EXISTS ENVIRONMENT_STAT;");
    execDB ("CREATE TABLE IF NOT EXISTS ENVIRONMENT_STAT (ID BIGINT PRIMARY KEY, "
            "DATA_SCALE INT, NUM_NODES INT, INSTANCE_TYPE VARCHAR(128), DISK_TYPE VARCHAR(128), "
            "MEM_SIZE BIGINT, NETWORK_TYPE VARCHAR(128), NUM_CPU INT) WITHOUT ROWID;");

    execDB ("DROP TABLE IF EXISTS QUERY_LAMBDA_STAT;");
    execDB ("CREATE TABLE IF NOT EXISTS QUERY_LAMBDA_STAT (ID BIGINT PRIMARY KEY, "
            "JOB_NAME VARCHAR(128), LAMBDA_ID BIGINT, ENVIRONMENT_ID BIGINT, "
            "DISTANCE INT, SELECTIVITY FLOAT, KEY_DISTRIBUTION BIGINT, "
            "FOREIGN KEY (LAMBDA_ID) REFERENCES LAMBDA_STAT(ID), "
            "FOREIGN KEY (ENVIRONMENT_ID) REFERENCES ENVIRONMENT_STAT(ID)) WITHOUT ROWID;");
 
    // to populate the environment table
    int environmentId = 0;
    int dataScale = 10;
    int numNodes = 5;
    std::string instanceType = "r2.4xlarge";
    std::string diskType = "SSD";
    size_t memSize = (size_t)(60) * (size_t)(1024) * (size_t)(1024) * (size_t)(1024);
    std::string networkType = "10Gb";
    int numCPU = 8;

    string cmdString = "INSERT INTO ENVIRONMENT_STAT "
                       "(ID, DATA_SCALE, NUM_NODES, INSTANCE_TYPE, DISK_TYPE, MEM_SIZE, NETWORK_TYPE, NUM_CPU) "
                       "VALUES (" + std::to_string(environmentId) + ","
                                  + std::to_string(dataScale) + ","
                                  + std::to_string(numNodes) + ",'"
                                  + instanceType + "','"
                                  + diskType + "',"
                                  + std::to_string(memSize) + ",'"
                                  + networkType + "',"
                                  + std::to_string(numCPU) + ");"; 
    std::cout << "Insert environment: " << cmdString << std::endl;
    execDB(cmdString);


    // to populate the lambda_stat table
    long lambdaId = 0;
    sqlite3_stmt * statement;
    std::string queryString = "SELECT DISTINCT JOB.NAME,LAMBDA.COMPUTATION_NAME,LAMBDA.LAMBDA_NAME,LAMBDA.LAMBDA_TYPE,"
               "LAMBDA.LAMBDA_IDENTIFIER,LAMBDA.LAMBDA_INPUT_CLASS FROM "
               " JOB JOIN LAMBDA ON JOB.ID = LAMBDA.JOB_ID";
    std::cout << "POPULATE LAMBDA_STAT: " << queryString << std::endl;
    std::vector<LambdaStruct_t> lambdas;
    std::map<std::string, std::vector<LambdaStruct_t>> lambdaMap;
    std::map<std::string, std::vector<LambdaStruct_t>> partitionLambdaMap;
    if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement,
               NULL) == SQLITE_OK) {
        int res;
        while ((res = sqlite3_step(statement)) == SQLITE_ROW) {
            LambdaStruct_t curLambda;
            curLambda.lambdaId = lambdaId;
            curLambda.jobName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
            curLambda.computationName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
            curLambda.lambdaName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
            curLambda.lambdaType = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
            curLambda.lambdaIdentifier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
            if(curLambda.lambdaType == "native_lambda") {
                curLambda.lambdaIdentifier = curLambda.jobName+"||"+curLambda.computationName+"||"+curLambda.lambdaName;
            }
            curLambda.typeName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
            if (curLambda.typeName == "tpch::Customer") {
               curLambda.typeName = "Customer";
            } else if (curLambda.typeName == "tpch::LineItem") {
               curLambda.typeName = "LineItem";
            } else if (curLambda.typeName == "tpch::Nation") {
               curLambda.typeName = "Nation";
            } else if (curLambda.typeName == "tpch::Order") {
               curLambda.typeName = "Order";
            } else if (curLambda.typeName == "tpch::Part") {
               curLambda.typeName = "Part";
            } else if (curLambda.typeName == "tpch::PartSupp") {
               curLambda.typeName = "PartSupp";
            } else if (curLambda.typeName == "tpch::Region") {
               curLambda.typeName = "Region";
            } else if (curLambda.typeName == "tpch::Supplier") {
               curLambda.typeName = "Supplier";
            } else {
               continue;
            }
            lambdaId ++;
            lambdas.push_back(curLambda);
            lambdaMap[curLambda.typeName].push_back(curLambda);
            partitionLambdaMap[curLambda.typeName+"||"+curLambda.lambdaType+"||"+curLambda.lambdaIdentifier].push_back(curLambda);
        }
    }
    
    std::cout << "there are " << lambdaMap.size() << " tables in total" << std::endl;
    for (auto a : lambdaMap) {
        std::cout << a.first << ":" << a.second.size() << std::endl;
    }

    std::cout << "there are " << partitionLambdaMap.size() << " partition lambdas in total" << std::endl;
    std::vector<LambdaStruct_t> partitionLambdas;
    std::map<std::string, std::vector<LambdaStruct_t>> partitionLambdaMapByTable;
    for (auto a : partitionLambdaMap) {
        std::cout << a.first << ":" << a.second.size() << std::endl;
        partitionLambdas.push_back(a.second[0]);
        partitionLambdaMapByTable[a.second[0].typeName].push_back(a.second[0]);
    }

    std::cout << "there are " << partitionLambdaMapByTable.size() << " tables in total" << std::endl;
    for (auto a : partitionLambdaMapByTable) {
        std::cout << a.first << ":" << a.second.size() << std::endl;
    }

    if (lambdas.size() == 0) {
        std::cerr << "No candidate lambdas to populate\n";
        exit(1);
    }

    cmdString = "INSERT INTO LAMBDA_STAT "
                       "(ID, LAMBDA_TYPE, LAMBDA_IDENTIFIER, JOB_NAME, COMPUTATION_NAME, LAMBDA_NAME, TYPE_NAME) "
                       "VALUES";
    cmdString += "(" + std::to_string(lambdas[0].lambdaId) + ",'"
                     + lambdas[0].lambdaType + "','"
                     + lambdas[0].lambdaIdentifier + "','"
                     + lambdas[0].jobName + "','"
                     + lambdas[0].computationName + "','"
                     + lambdas[0].lambdaName + "','"
                     + lambdas[0].typeName + "')";          
    for (size_t i = 1; i < lambdas.size(); i++) {
        cmdString += ", (" + std::to_string(lambdas[i].lambdaId) + ",'"
                     + lambdas[i].lambdaType + "','"
                     + lambdas[i].lambdaIdentifier + "','"
                     + lambdas[i].jobName + "','"         
                     + lambdas[i].computationName + "','"   
                     + lambdas[i].lambdaName + "','"      
                     + lambdas[i].typeName + "')";  
    } 
    cmdString += ";";
    std::cout << "Insert lambdas: " << cmdString << std::endl;
    execDB(cmdString);

    lambdas.clear();
    lambdaMap.clear();

    LambdaStruct_t curQueryLambda;
    //populate query lambda statistics
    cmdString = "INSERT INTO QUERY_LAMBDA_STAT "
            "(ID, JOB_NAME, LAMBDA_ID, ENVIRONMENT_ID, DISTANCE, SELECTIVITY, KEY_DISTRIBUTION) "
            "VALUES";
    long curQueryLambdaId = 0;
    for (auto a : partitionLambdaMap) {
        std::cout << "############to populate query-lambda statistics for lambda with id="
                  << a.second[0].lambdaId 
                  << " and name="
                  << a.first << std::endl;

        //find out input data size
        std::string curLambdaClass = std::string("tpch::")+a.second[0].typeName;
        size_t inputSize = 0;
        queryString = "SELECT DATA.SIZE FROM DATA "
                      "WHERE DATA.CLASS_NAME='"+curLambdaClass+"'";
        std::cout << "SELECT DATA SIZE: " << queryString << std::endl;
        if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement, NULL) == SQLITE_OK) {
            int res = sqlite3_step(statement);
            if (res == SQLITE_ROW) {
                inputSize = sqlite3_column_int64(statement, 0);
            }
        }
        std::cout << "input size is " << inputSize << std::endl;

        for (size_t i = 0; i < a.second.size(); i++) {
            curQueryLambda = a.second[i];
            std::cout << "to populate query-lambda statistics for query=" << curQueryLambda.jobName
                      << ", computation=" << curQueryLambda.computationName << ", lambdaName="
                      << curQueryLambda.lambdaName << ", lambdaId=" << curQueryLambda.lambdaId << std::endl;

            //find out input data size
            std::string curLambdaClass = std::string("tpch::")+curQueryLambda.typeName;
            size_t inputSize = 0;
            queryString = "SELECT DATA.SIZE FROM DATA " 
                          "WHERE DATA.CLASS_NAME='"+curLambdaClass+"'";
            if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement, NULL) == SQLITE_OK) {
                int res = sqlite3_step(statement);
                if (res == SQLITE_ROW) {
                   inputSize = sqlite3_column_int64(statement, 0);
                }
            }
            std::cout << "input size is " << inputSize << std::endl;                  

            //find out job stage id and key distribution
            int numHashKeys = 0;
            int jobStageId = 0;
            queryString = "SELECT JOB_STAGE.ID, JOB_STAGE.NUM_HASH_KEYS FROM JOB JOIN JOB_INSTANCE ON "
                          "JOB.ID = JOB_INSTANCE.JOB_ID AND JOB.NAME='"+curQueryLambda.jobName+"' " 
                          "JOIN JOB_STAGE ON "
                          "JOB_INSTANCE.ID = JOB_STAGE.JOB_INSTANCE_ID AND "
                          "JOB_STAGE.TARGET_COMPUTATION_SPECIFIER='"+curQueryLambda.computationName+"' ";
            std::cout << "To identify the job stage that applies this lambda:" << queryString << std::endl;
            if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement, NULL) == SQLITE_OK) {
                int res = sqlite3_step(statement);
                if (res == SQLITE_ROW) {
                   jobStageId = sqlite3_column_int64(statement, 0);
                   numHashKeys = sqlite3_column_int(statement, 1);
                }
            }
            std::cout << "stage Id is " << jobStageId << std::endl;
            std::cout << "numHashKeys is " << numHashKeys << std::endl; 
                          
            //find out output size
            size_t outputSize = 0;
            queryString = "SELECT DATA.SIZE FROM DATA JOIN DATA_JOB_STAGE ON "
                          "DATA.ID = DATA_JOB_STAGE.DATA_ID AND DATA_JOB_STAGE.DATA_TYPE = 'Sink' AND "
                          "DATA_JOB_STAGE.JOB_STAGE_ID = " + std::to_string(jobStageId);
            std::cout << "to identify the output size: " << queryString << std::endl;
            if (sqlite3_prepare_v2(selfLearningDBHandler, queryString.c_str(), -1, &statement, NULL) == SQLITE_OK) {
                int res = sqlite3_step(statement);
                if (res == SQLITE_ROW) {
                   outputSize = sqlite3_column_int64(statement, 0);
                }
            }
            std::cout << "output size is " << outputSize << std::endl;

            //find out selectivity
            double selectivity = (double)(outputSize)/(double)(inputSize);
            std::cout << "selectivity is " << selectivity << std::endl;            

            //find out distance  
            int distance = 1;
            std::cout << "distance is " << distance << std::endl;
            
            if (curQueryLambdaId == 0) {
                cmdString += "(" + std::to_string(curQueryLambdaId) + ",'"
                                 + curQueryLambda.jobName + "'," 
                                 + std::to_string(a.second[0].lambdaId)+ ",0,"
                                 + std::to_string(distance) + ","
                                 + std::to_string(selectivity) + ","
                                 + std::to_string(numHashKeys) + ")"; 
            } else {
                cmdString += ",(" + std::to_string(curQueryLambdaId) + ",'"
                                 + curQueryLambda.jobName + "',"
                                 + std::to_string(a.second[0].lambdaId)+ ",0,"
                                 + std::to_string(distance) + ","
                                 + std::to_string(selectivity) + ","
                                 + std::to_string(numHashKeys) + ")"; 
            }
            curQueryLambdaId ++;
        }
    }
    cmdString += ";";
    std::cout << "Insert query lambda stat: " << cmdString << std::endl;
    execDB(cmdString);

    //populate partition lambdas (with redundant lambdas identified and removed)
    cmdString = "INSERT INTO PARTITION_LAMBDA_STAT "
                       "(ID, LAMBDA_TYPE, LAMBDA_IDENTIFIER, JOB_NAME, COMPUTATION_NAME, LAMBDA_NAME, TYPE_NAME) VALUES";
    cmdString += "(" + std::to_string(partitionLambdas[0].lambdaId) + ",'"
                     + partitionLambdas[0].lambdaType + "','"
                     + partitionLambdas[0].lambdaIdentifier + "','"
                     + partitionLambdas[0].jobName + "','"
                     + partitionLambdas[0].computationName + "','"
                     + partitionLambdas[0].lambdaName + "','"
                     + partitionLambdas[0].typeName + "')";
    for (size_t i = 1; i < lambdas.size(); i++) {
        cmdString += ", (" + std::to_string(partitionLambdas[i].lambdaId) + ",'"
                     + partitionLambdas[i].lambdaType + "','"
                     + partitionLambdas[i].lambdaIdentifier + "','"
                     + partitionLambdas[i].jobName + "','"
                     + partitionLambdas[i].computationName + "','"
                     + partitionLambdas[i].lambdaName + "','"
                     + partitionLambdas[i].typeName + "')";
    }
    cmdString += ";";
    std::cout << "Insert partition lambdas: " << cmdString << std::endl;
    execDB(cmdString);


    // to populate the partition scheme
    int customerLambdaIndex = -1;
    int lineitemLambdaIndex = -1;
    int nationLambdaIndex = -1;
    int orderLambdaIndex = -1;        
    int partLambdaIndex = -1;
    int partsuppLambdaIndex = -1;
    int regionLambdaIndex = -1;
    int supplierLambdaIndex = -1;

    int curCustomerIndex = -1;
    int curLineItemIndex = -1;
    int curNationIndex = -1;
    int curOrderIndex = -1;
    int curPartIndex = -1;
    int curPartSuppIndex = -1;
    int curRegionIndex = -1;
    int curSupplierIndex = -1;

    cmdString = "INSERT INTO PARTITION_SCHEME_STAT "
                       "(ID, CUSTOMER_LAMBDA_ID, LINEITEM_LAMBDA_ID, NATION_LAMBDA_ID, ORDER_LAMBDA_ID, "
                       "PART_LAMBDA_ID, PARTSUPP_LAMBDA_ID, "
                       "REGION_LAMBDA_ID, SUPPLIER_LAMBDA_ID) VALUES ";
    long partitionSchemeId = 0;    
    for (customerLambdaIndex = -1; customerLambdaIndex < (int)(partitionLambdaMapByTable["Customer"].size()); customerLambdaIndex++) {
        for (lineitemLambdaIndex = -1; lineitemLambdaIndex < (int)(partitionLambdaMapByTable["LineItem"].size()); lineitemLambdaIndex++) {
            for (nationLambdaIndex = -1; nationLambdaIndex < (int)(partitionLambdaMapByTable["Nation"].size()); nationLambdaIndex++) {
                for (orderLambdaIndex = -1; orderLambdaIndex < (int)(partitionLambdaMapByTable["Order"].size()); orderLambdaIndex++) {
                     for (partLambdaIndex = -1; partLambdaIndex < (int)(partitionLambdaMapByTable["Part"].size()); partLambdaIndex++) {
                         for (partsuppLambdaIndex = -1; partsuppLambdaIndex < (int)(partitionLambdaMapByTable["PartSupp"].size()); partsuppLambdaIndex++) {
                             for (regionLambdaIndex = -1; regionLambdaIndex < (int)(partitionLambdaMapByTable["Region"].size()); regionLambdaIndex++) {
                                 for (supplierLambdaIndex = -1; supplierLambdaIndex < (int)(partitionLambdaMapByTable["Supplier"].size()); supplierLambdaIndex++) {
                                     if (customerLambdaIndex >= 0) {
                                          curCustomerIndex = partitionLambdaMapByTable["Customer"][customerLambdaIndex].lambdaId;
                                     } else {
                                          curCustomerIndex = -1;
                                     } 
                                     if (lineitemLambdaIndex >= 0) {
                                          curLineItemIndex = partitionLambdaMapByTable["LineItem"][lineitemLambdaIndex].lambdaId;
                                     } else {
                                          curLineItemIndex = -1;
                                     }
                                     if (nationLambdaIndex >= 0) {
                                          curNationIndex = partitionLambdaMapByTable["Nation"][nationLambdaIndex].lambdaId;
                                     } else {
                                          curNationIndex = -1;
                                     }
                                     if (orderLambdaIndex >= 0) {
                                          curOrderIndex = partitionLambdaMapByTable["Order"][orderLambdaIndex].lambdaId;
                                     } else {
                                          curOrderIndex = -1;
                                     }
                                     if (partLambdaIndex >= 0) {
                                          curPartIndex = partitionLambdaMapByTable["Part"][partLambdaIndex].lambdaId;
                                     } else {
                                          curPartIndex = -1;
                                     }
                                     if (partsuppLambdaIndex >= 0) {
                                          curPartSuppIndex = partitionLambdaMapByTable["PartSupp"][partsuppLambdaIndex].lambdaId;
                                     } else {
                                          curPartSuppIndex = -1;
                                     }
                                     if (regionLambdaIndex >= 0) {
                                          curRegionIndex = partitionLambdaMapByTable["Region"][regionLambdaIndex].lambdaId;
                                     } else {
                                          curRegionIndex = -1;
                                     }                   
                                     if (supplierLambdaIndex >= 0) {
                                          curSupplierIndex = partitionLambdaMapByTable["Supplier"][supplierLambdaIndex].lambdaId;
                                     } else {
                                          curSupplierIndex = -1;
                                     }
                                     if (partitionSchemeId == 0) {
                                         cmdString += "(" + std::to_string(partitionSchemeId) + ","
                                                          + std::to_string(curCustomerIndex) + ","
                                                          + std::to_string(curLineItemIndex) + ","
                                                          + std::to_string(curNationIndex) + ","
                                                          + std::to_string(curOrderIndex) + ","
                                                          + std::to_string(curPartIndex) + ","
                                                          + std::to_string(curPartSuppIndex) + ","
                                                          + std::to_string(curRegionIndex) + ","
                                                          + std::to_string(curSupplierIndex) + ")";

                                     } else {
                                         cmdString += ",("+ std::to_string(partitionSchemeId) + "," 
                                                          + std::to_string(curCustomerIndex) + ","
                                                          + std::to_string(curLineItemIndex) + ","
                                                          + std::to_string(curNationIndex) + ","
                                                          + std::to_string(curOrderIndex) + ","
                                                          + std::to_string(curPartIndex) + ","
                                                          + std::to_string(curPartSuppIndex) + ","
                                                          + std::to_string(curRegionIndex) + ","
                                                          + std::to_string(curSupplierIndex) + ")";
                                    }
                                    partitionSchemeId ++; 
                                 }
                             }
                         }
                     }
                }
            }
        }
    }
    cmdString +=";";
    std::cout << "Insert partition schemes: " << cmdString << std::endl;
    execDB(cmdString);
    sqlite3_close_v2(selfLearningDBHandler);
    
}

#endif
