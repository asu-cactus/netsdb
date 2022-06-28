#ifndef HERMES_EXECUTION_SERVER_H
#define HERMES_EXECUTION_SERVER_H

#include "PDBDebug.h"
#include "ServerFunctionality.h"
#include "SharedMem.h"
#include "Configuration.h"
#include "PageScanner.h"
#include "DataTypes.h"
#include "HashSetManager.h"
#include "DataProxy.h"
#include <string>

namespace pdb {

// this server functionality is supposed to run in backend
// a HermesExecutionServer runs in backend for distributed query execution.
// the QuerySchedulerServer split a query job into mulitple JobStages and dispatch
// those JobStages to HermesExecutionServer for execution.

// There are four different types of JobStages in total:
// -- TupleSetJobStage: this encapsulates a pipeline of computations;
// -- AggregationJobStage: this encapsulates the final aggregation execution that
//    receives data from a shuffled set, and build multiple hash tables over the data;
// -- BroadcastJoinBuildHTJobStage: this encapsulates the execution of hash table building
//    for broadcast join;
// -- PartitionedJoinBuildHTJobStage: this encapsulates the execution of hash table 
//    building for hash partitioned join.

// HermesExecutionServer executes each of those JobStages and 
// stores intermediate data in local storage.



class HermesExecutionServer : public ServerFunctionality {

public:
    // creates an execution server...
    HermesExecutionServer(int nodeId, 
                          SharedMemPtr shm,
                          PDBWorkerQueuePtr workers,
                          PDBLoggerPtr logger,
                          ConfigurationPtr conf,
                          bool selfLearningOrNot = false) {
        this->shm = shm;
        this->conf = conf;
        this->nodeId = nodeId;
        this->curScanner = nullptr;
        this->logger = logger;
        this->workers = workers;
        this->selfLearningOrNot = selfLearningOrNot;
    }


    // print cache stats in frontend
    void printCacheStats () {
        std::string errMsg;
        PDBCommunicatorPtr anotherCommunicatorToFrontend = make_shared<PDBCommunicator>();
        anotherCommunicatorToFrontend->connectToInternetServer(logger, conf->getPort(), conf->getServerAddress(), errMsg);
        DataProxyPtr proxy = make_shared<DataProxy>(nodeId, anotherCommunicatorToFrontend, shm, logger);
        proxy->printStats(true);
    }


    // set the configuration instance;
    void setConf(ConfigurationPtr conf) {
        this->conf = conf;
    }

    // return the configuration instance;
    ConfigurationPtr getConf() {
        return this->conf;
    }

    // set the shared memory instance;
    void setSharedMem(SharedMemPtr shm) {
        this->shm = shm;
    }

    // return the shared memory instance;
    SharedMemPtr getSharedMem() {
        return this->shm;
    }

    // set the nodeId for this backend server;
    void setNodeID(NodeID nodeId) {
        this->nodeId = nodeId;
    }

    // return the nodeId of this backend server;
    NodeID getNodeID() {
        return this->nodeId;
    }


    // from the ServerFunctionality interface... registers the HermesExecutionServer's        //
    // handlers
    void registerHandlers(PDBServer& forMe) override;

    // register the PageScanner for current job;
    // now only one job is allowed to run in an execution server at a time
    bool setCurPageScanner(PageScannerPtr curPageScanner) {
        if (curPageScanner == nullptr) {
            this->curScanner = nullptr;
            return true;
        }

        if (this->curScanner == nullptr) {
            this->curScanner = curPageScanner;
            PDB_COUT << "scanner set for current job\n";
            return true;
        } else {
            // a job is already running
            cout << "PDBBackEnd: a job is already running...\n";
            return false;
        }
    }


    // return the PageScanner of current job;
    PageScannerPtr getCurPageScanner() {
        return this->curScanner;
    }

    // set the logger
    void setLogger(pdb::PDBLoggerPtr logger) {
        this->logger = logger;
    }

    // get the logger
    pdb::PDBLoggerPtr getLogger() {
        return this->logger;
    }

    // set the workers
    void setWorkers(PDBWorkerQueuePtr workers) {
        this->workers = workers;
    }

    // get the workers
    PDBWorkerQueuePtr getWorkers() {
        return this->workers;
    }

    // destructor
    ~HermesExecutionServer() {}

    // get hash set
    AbstractHashSetPtr getHashSet(std::string name) {
        return this->hashSetMgr.getHashSet(name);
    }

    // add hash set
    bool addHashSet(std::string name, AbstractHashSetPtr hashSet) {
        return this->hashSetMgr.addHashSet(name, hashSet);
    }

    // remove hash set
    bool removeHashSet(std::string name) {
        return this->hashSetMgr.removeHashSet(name);
    }

    size_t getHashSetsSize() {

        return this->hashSetMgr.getTotalSize();
    }

    void setSelfLearning (bool selfLearningOrNot) {

        this->selfLearningOrNot = selfLearningOrNot;

    }

    bool isSelfLearning () {

        return this->selfLearningOrNot;

    }

    /*
     * We plan to support three materialization types:
     * (1) "compiled": the model is compiled to assembly code
     * (2) "fast": the model is stored to facilitate the RapidScorer algorithm
     * (3) "naive": the model is materialized for naive tree traversal
     * We plan to support following models:
     * (1) "randomforest"
     * (2) "gdbt"
     * (3) "conv"
     * (4) "ffnn"
     */
    void * materializeModel(std::string pathToModel, std::string modelMaterializationType, std::string modelType, std::string & errMsg);


private:
    ConfigurationPtr conf;
    SharedMemPtr shm;
    PDBWorkerQueuePtr workers;
    NodeID nodeId;
    PageScannerPtr curScanner;
    pdb::PDBLoggerPtr logger;
    HashSetManager hashSetMgr;
    bool selfLearningOrNot;
};
}

#endif
