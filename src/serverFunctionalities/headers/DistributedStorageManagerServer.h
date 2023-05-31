#ifndef OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERSERVER_H
#define OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERSERVER_H

#include "PDBLogger.h"

#include "BroadcastServer.h"
#include "SimpleRequestResult.h"
#include "StorageAddModelResponse.h"

namespace pdb {

// This ServerFunctionality runs on the Master server, and talks with
// the PangeaStorageServer deployed on each Worker server to serve
// storage-related requests. Include following:
// -- DistributedStorageAddDatabase: to add a database over the cluster
// -- DistributedStorageClearSet: to remove data (both in-memory or on-disk data) from a set
// -- DistributedStorageAddTempSet: to add a temp set (invisible to catalog) over the cluster
// -- DistributedStorageAddSet: to add a user set over the cluster
// -- DistributedStorageRemoveDatabase: to remove a database from the cluster
// -- DistributedStorageRemoveTempSet: to remove a temp set (invisible to catalog) 
//    from the cluster
// -- DistributedStorageRemoveSet: to remove a user set over the cluster
// -- DistributedStorageCleanup: to flush buffered data to disk for all storage nodes
// -- DistributedStorageExportSet: to export a set to user specified format
// -- SetScan: to scan all pages in a set from the client (One thread)



class DistributedStorageManagerServer : public BroadcastServer {

public:
    DistributedStorageManagerServer(PDBLoggerPtr logger, ConfigurationPtr conf, bool selfLearningOrNot = false, bool trainingMode = false);


    DistributedStorageManagerServer(PDBLoggerPtr logger, bool selfLearningOrNot = false, bool trainingMode = false);
    ~DistributedStorageManagerServer();

    void setSelfLearning (bool selfLearningOrNot) {

        this->selfLearningOrNot = selfLearningOrNot;

    }

    bool isSelfLearning () {

        return this->selfLearningOrNot;

    }

    static std::string getNextClientId() {
        time_t currentTime = time(NULL);
        struct tm* local = localtime(&currentTime);
        std::string clientId = "Client-" + std::to_string(local->tm_year + 1900) + "_" +
            std::to_string(local->tm_mon + 1) + "_" + std::to_string(local->tm_mday) + "_" +
            std::to_string(local->tm_hour) + "_" + std::to_string(local->tm_min) + "_" +
            std::to_string(local->tm_sec);
        return clientId;
    }

    long getIdForData(std::string databaseName, std::string setName) {
        if (idMap.count(std::pair<std::string, std::string>(databaseName, setName)) > 0) {
             return idMap[std::pair<std::string, std::string>(databaseName, setName)];
        } else {
             return -1;
        }
    }

    void removeData(std::string databaseName, std::string setName) {

        idMap.erase(std::pair<std::string, std::string>(databaseName, setName));

    }

    void registerHandlers(PDBServer& forMe) override;

    void getModelPointers(std::string pathToModel,  std::vector<void *> &modelPointers) {
    
	    if (modelMap.count(pathToModel) > 0)
		    modelPointers = modelMap[pathToModel];
    }

private:
    bool findNodesForDatabase(const std::string& databaseName,
                              std::vector<std::string>& nodesForDatabase,
                              std::string& errMsg);

    bool findNodesContainingDatabase(const std::string& databaseName,
                                     std::vector<std::string>& nodesContainingDatabase,
                                     std::string& errMsg);

    bool findNodesForSet(const std::string& databaseName,
                         const std::string& setName,
                         std::vector<std::string>& nodesContainingSet,
                         std::string& errMsg);

    bool findNodesContainingSet(const std::string& databaseName,
                                const std::string& setName,
                                std::vector<std::string>& nodesContainingSet,
                                std::string& errMsg);

    std::function<void(Handle<SimpleRequestResult>, std::string)> generateAckHandler(
        std::vector<std::string>& success, std::vector<std::string>& failures, mutex& lock);


    //handler for handle each response to the StorageAddModel request
    std::function<void(Handle<StorageAddModelResponse>, std::string)> generateResponseHandler(
        std::vector<std::string>& success, std::vector<std::string>& failures, mutex& lock, std::vector<void *> &pointers, 
	std::vector<std::string>& nodesToBroadcast);

    bool selfLearningOrNot;


    bool trainingOrNot;


    //a map from database name, set name to id in DATA table
    std::map<std::pair<std::string, std::string>, long> idMap; 


    //a map maps model path to the pointers to the materialized models in the backends of workers
    std::map<std::string, std::vector<void *>> modelMap;

};
}


#endif  // OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERSERVER_H
