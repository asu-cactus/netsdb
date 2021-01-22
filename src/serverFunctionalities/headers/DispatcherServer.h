
#ifndef OBJECTQUERYMODEL_DISPATCHER_H
#define OBJECTQUERYMODEL_DISPATCHER_H

#include "ServerFunctionality.h"
#include "PDBLogger.h"
#include "PDBWork.h"
#include "PartitionPolicy.h"
#include "UseTemporaryAllocationBlock.h"
#include "PDBVector.h"

#include "NodeDispatcherData.h"
#include "StorageClient.h"

#include <string>
#include <queue>
#include <unordered_map>
#include <vector>



namespace pdb {

// The DispatcherServer partitions and then forwards a Vector of pdb::Objects received from a
// DispatcherClient to the proper storage servers
// So far, there are two dispatching policies are supported:
// -- Random Policy: the received Vector will be sent to any storage node determined randomly
// -- Round-Robin Policy: the first received Vector will be sent to the first storage node, 
//    and so on.


class DispatcherServer : public ServerFunctionality {

public:
    DispatcherServer(PDBLoggerPtr logger, bool selfLearningOrNot = false);

    ~DispatcherServer();

    void initialize();

    /**
     * Inherited function from ServerFunctionality
     * @param forMe
     */
    void registerHandlers(PDBServer& forMe) override;

    /**
     * Updates PartitionPolicy with a collection of all the available storage nodes in the cluster
     *
     * @param storageNodes a vector of the live storage nodes
     */
    void registerStorageNodes(Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);

    /**
     * Register a new set for the dispatcher to handle.
     *
     * @param setAndDatabase name of the set and its corresponding database
     * @param partitionPolicy policy by which to partition data for this set
     */
    void registerSet(std::pair<std::string, std::string> setAndDatabase,
                     PartitionPolicyPtr partitionPolicy);

    /**
     * Deregister a set from the dispatcher (often due to set removal)
     */
    void deregisterSet(std::pair<std::string, std::string> setAndDatabase);




    /**
     * Dispatch a Vector of pdb::Object's to the correct StorageNodes as defined by that particular
     * set's ParitionPolicy
     *
     * @param setAndDatabase name of the set and its corresponding database
     * @param toDispatch vector of pdb::Object's to dispatch
     * @return true on success
     */
    bool dispatchData(std::pair<std::string, std::string> setAndDatabase,
                      std::string type,
                      Handle<Vector<Handle<Object>>> toDispatch);
    bool dispatchBytes(std::pair<std::string, std::string> setAndDatabase,
                       std::string type,
                       char* bytes,
                       size_t numBytes);


    void waitAllRequestsProcessed() {
        pthread_mutex_lock(&mutex);
        int numRequests = numRequestsInProcessing;
        pthread_mutex_unlock(&mutex);
        std::cout << "numRequestsInProcessing: " << numRequests << std::endl;
        while (numRequests > 0) {
            sleep(1);
            pthread_mutex_lock(&mutex);
            numRequests = numRequestsInProcessing;
            pthread_mutex_unlock(&mutex);
            std::cout << "numRequestsInProcessing: " << numRequests << std::endl;
        }
    }

    void setSelfLearning (bool selfLearningOrNot) {

        this->selfLearningOrNot = selfLearningOrNot;

    }

    bool isSelfLearning () {

        return this->selfLearningOrNot;

    }


private:
    PDBLoggerPtr logger;
    Handle<Vector<Handle<NodeDispatcherData>>> storageNodes;
    std::map<std::pair<std::string, std::string>, PartitionPolicyPtr> partitionPolicies;

    /**
     * Validates with the catalog that a request to store data is correct
     * @return true if the type matches the known set
     */
    bool validateTypes(const std::string& databaseName,
                       const std::string& setName,
                       const std::string& typeName,
                       std::string& errMsg);

    bool sendData(std::pair<std::string, std::string> setAndDatabase,
                  std::string type,
                  Handle<NodeDispatcherData> destination,
                  Handle<Vector<Handle<Object>>> toSend);

    bool sendBytes(std::pair<std::string, std::string> setAndDatabase,
                   std::string type,
                   Handle<NodeDispatcherData> destination,
                   char* bytes,
                   size_t numBytes);

    Handle<NodeDispatcherData> findNode(NodeID nodeId);
    int numRequestsInProcessing = 0;
    pthread_mutex_t mutex;
    bool selfLearningOrNot;
};
}


#endif  // OBJECTQUERYMODEL_DISPATCHER_H
