#ifndef OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERSERVER_CC
#define OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERSERVER_CC

#include "PDBDebug.h"
#include "DistributedStorageManagerServer.h"
#include "CatalogClient.h"
#include "CatalogServer.h"
#include "ResourceManagerServer.h"
#include "DispatcherServer.h"
#include "PDBCatalogMsgType.h"
#include "SimpleRequestHandler.h"

#include "DistributedStorageAddDatabase.h"
#include "DistributedStorageAddSet.h"
#include "DistributedStorageAddSetWithPartition.h"
#include "DistributedStorageAddTempSet.h"
#include "DistributedStorageRemoveDatabase.h"
#include "DistributedStorageRemoveSet.h"
#include "DistributedStorageRemoveTempSet.h"
#include "DistributedStorageExportSet.h"
#include "DistributedStorageClearSet.h"
#include "DistributedStorageCleanup.h"
#include "Computation.h"
#include "ComputationNode.h"
#include "QuerySchedulerServer.h"
#include "DispatcherServer.h"
#include "Lambda.h"
#include "LambdaPolicy.h"
#include "IRPolicy.h"
#include "Statistics.h"
#include "StorageAddDatabase.h"
#include "StorageAddSet.h"
#include "StorageAddTempSet.h"
#include "StorageRemoveDatabase.h"
#include "StorageRemoveUserSet.h"
#include "StorageExportSet.h"
#include "StorageClearSet.h"
#include "StorageCleanup.h"
#include "Configuration.h"
#include "SelfLearningServer.h"
#include "SetScan.h"
#include "KeepGoing.h"
#include "DoneWithResult.h"
#include "RuleBasedDataPlacementOptimizerForLoadJob.h"
#include "DRLBasedDataPlacementOptimizerForLoadJob.h"

#include <chrono>
#include <ctime>
#include <unistd.h>
#include <snappy.h>

#define USING_ALL_NODES

namespace pdb {

DistributedStorageManagerServer::DistributedStorageManagerServer(PDBLoggerPtr logger,
                                                                 ConfigurationPtr conf,
                                                                 bool selfLearningOrNot,
                                                                 bool trainingOrNot)
    : BroadcastServer(logger, conf) {
    this->selfLearningOrNot = selfLearningOrNot;
    this->trainingOrNot = trainingOrNot;
}


DistributedStorageManagerServer::DistributedStorageManagerServer(PDBLoggerPtr logger, bool selfLearningOrNot, bool trainingOrNot)
    : BroadcastServer(logger) {
    this->selfLearningOrNot = selfLearningOrNot;
    this->trainingOrNot = trainingOrNot;
}


DistributedStorageManagerServer::~DistributedStorageManagerServer() {
    // no-op
}

void DistributedStorageManagerServer::registerHandlers(PDBServer& forMe) {

    /**
     * Handler that distributes an add database request
     */
    forMe.registerHandler(
        DistributedStorageAddDatabase_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageAddDatabase>>(
            [&](Handle<DistributedStorageAddDatabase> request, PDBCommunicatorPtr sendUsingMe) {
                const UseTemporaryAllocationBlock tempBlock{1 * 1024 * 1024};
                std::string errMsg;
                std::string database = request->getDatabase();
                std::string value;
                int catalogType = PDBCatalogMsgType::CatalogPDBDatabase;

                if (getFunctionality<CatalogClient>().databaseExists(database)) {
                    PDB_COUT << "Database " << database << " already exists " << std::endl;
                } else {
                    PDB_COUT << "Database " << database << " does not exist" << std::endl;
                    if (!getFunctionality<CatalogClient>().createDatabase(database, errMsg)) {
                        std::cout << "Could not register db, because: " << errMsg << std::endl;
                        Handle<SimpleRequestResult> response =
                            makeObject<SimpleRequestResult>(false, errMsg);
                        bool res = sendUsingMe->sendObject(response, errMsg);
                        return make_pair(res, errMsg);
                    }
                }

                mutex lock;
                auto successfulNodes = std::vector<std::string>();
                auto failureNodes = std::vector<std::string>();
                auto nodesToBroadcastTo = std::vector<std::string>();

#ifndef USING_ALL_NODES
                if (!getFunctionality<DistributedStorageManagerServer>().findNodesForDatabase(
                        database, nodesToBroadcastTo, errMsg)) {
                    PDB_COUT << "Could not find nodes to broadcast database to: " << errMsg
                             << std::endl;
                    Handle<SimpleRequestResult> response =
                        makeObject<SimpleRequestResult>(false, errMsg);
                    bool res = sendUsingMe->sendObject(response, errMsg);
                    return make_pair(res, errMsg);
                }
#else
                std::vector<std::string> allNodes;
                const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
                for (int i = 0; i < nodes->size(); i++) {
                    std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                    std::string port = std::to_string((*nodes)[i]->getPort());
                    allNodes.push_back(address + ":" + port);
                }
                nodesToBroadcastTo = allNodes;
#endif

                Handle<StorageAddDatabase> storageCmd =
                    makeObject<StorageAddDatabase>(request->getDatabase());
                getFunctionality<DistributedStorageManagerServer>()
                    .broadcast<StorageAddDatabase, Object, SimpleRequestResult>(
                        storageCmd,
                        nullptr,
                        nodesToBroadcastTo,
                        generateAckHandler(successfulNodes, failureNodes, lock),
                        [&](std::string errMsg, std::string serverName) {
                            lock.lock();
                            std::cout << "Server " << serverName << " received an error: " << errMsg
                                      << std::endl;
                            failureNodes.push_back(serverName);
                            lock.unlock();
                        });

                bool res = true;
                Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
                res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }));

    forMe.registerHandler(
        DistributedStorageClearSet_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageClearSet>>(
            [&](Handle<DistributedStorageClearSet> request, PDBCommunicatorPtr sendUsingMe) {
                const UseTemporaryAllocationBlock tempBlock{8 * 1024 * 1024};
                std::cout << "received DistributedStorageClearSet message" << std::endl;
                std::string errMsg;
                bool res = true;
                mutex lock;

                auto successfulNodes = std::vector<std::string>();
                auto failureNodes = std::vector<std::string>();
                auto nodesToBroadcast = std::vector<std::string>();

                std::string database = request->getDatabase();
                std::string set = request->getSetName();

                if (getFunctionality<CatalogClient>().setExists(
                        database, set)) {
// to remove set
                    std::vector<std::string> allNodes;
                    const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
                    for (int i = 0; i < nodes->size(); i++) {
                        std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                        std::string port = std::to_string((*nodes)[i]->getPort());
                        allNodes.push_back(address + ":" + port);
                    }
                    nodesToBroadcast = allNodes;
                    Handle<StorageClearSet> storageCmd = makeObject<StorageClearSet>(
                        request->getDatabase(), request->getSetName(), request->getTypeName());


                    getFunctionality<DistributedStorageManagerServer>()
                        .broadcast<StorageClearSet, Object, SimpleRequestResult>(
                            storageCmd,
                            nullptr,
                            nodesToBroadcast,
                            generateAckHandler(successfulNodes, failureNodes, lock));
                    res = true;
                } else {
                    res = false;
                    errMsg = std::string("Set to clear with name=") + database + ":" + set  +
                        std::string(" doesn't exist");
                }

                // update stats
                StatisticsPtr stats = getFunctionality<QuerySchedulerServer>().getStats();
                if (stats == nullptr) {
                    getFunctionality<QuerySchedulerServer>().collectStats();
                    stats = getFunctionality<QuerySchedulerServer>().getStats();
                }
                stats->setNumPages(request->getDatabase(), request->getSetName(), 0);
                stats->setNumBytes(request->getDatabase(), request->getSetName(), 0);

                Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
                res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }));


    forMe.registerHandler(
        DistributedStorageAddTempSet_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageAddTempSet>>([&](
            Handle<DistributedStorageAddTempSet> request, PDBCommunicatorPtr sendUsingMe) {
            const UseTemporaryAllocationBlock tempBlock{8 * 1024 * 1024};
            auto begin = std::chrono::high_resolution_clock::now();

            PDB_COUT << "received DistributedStorageAddTempSet message" << std::endl;
            std::string errMsg;
            mutex lock;

            auto successfulNodes = std::vector<std::string>();
            auto failureNodes = std::vector<std::string>();
            auto nodesToBroadcast = std::vector<std::string>();

            std::string set = request->getSetName();
            std::string value;

            std::vector<std::string> allNodes;
            const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
            for (int i = 0; i < nodes->size(); i++) {
                std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                std::string port = std::to_string((*nodes)[i]->getPort());
                allNodes.push_back(address + ":" + port);
            }
            nodesToBroadcast = allNodes;
            size_t desiredSize = request->getDesiredSize();
            if (desiredSize == 0) {
                desiredSize = 1;
            }
            std::cout << "******************** desired size = " << desiredSize << "********************" << std::endl;
            Handle<StorageAddSet> storageCmd = makeObject<StorageAddSet>(request->getDatabaseName(),
                                                                         request->getSetName(),
                                                                         request->getTypeName(),
                                                                         request->getPageSize(),
                                                                         desiredSize,
                                                                         true,
                                                                         true);

            getFunctionality<DistributedStorageManagerServer>()
                .broadcast<StorageAddSet, Object, SimpleRequestResult>(
                    storageCmd,
                    nullptr,
                    nodesToBroadcast,
                    generateAckHandler(successfulNodes, failureNodes, lock));

            auto storageAddSetEnd = std::chrono::high_resolution_clock::now();
            PDB_COUT << "Time Duration for adding temp set:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(storageAddSetEnd -
                                                                                 begin)
                            .count()
                     << " secs." << std::endl;

            bool res = true;
            if (failureNodes.size() > 0) {
                res = false;
                errMsg = "";
                for (int i = 0; i < failureNodes.size(); i++) {
                    errMsg += failureNodes[i] + std::string(";");
                }
            }
            // update stats
            StatisticsPtr stats = getFunctionality<QuerySchedulerServer>().getStats();
            if (stats == nullptr) {
                getFunctionality<QuerySchedulerServer>().collectStats();
                stats = getFunctionality<QuerySchedulerServer>().getStats();
            }
            stats->setNumPages("temp", request->getSetName(), 0);
            stats->setNumBytes("temp", request->getSetName(), 0);
            if (this->selfLearningOrNot == true) {
                long id;
                int typeId = VTableMap::getIDByName(VTableMap::getInternalTypeName(request->getTypeName()));
                std::string createdJobId = request->getCreatedJobId();
                if (createdJobId == "") {
                    createdJobId = this->getNextClientId();
                }
                std::cout << "to create data in sqlite database for " << request->getDatabaseName() << ":" << request->getSetName() << std::endl;
                getFunctionality<SelfLearningServer>().createData(request->getDatabaseName(), request->getSetName(),
                                                            createdJobId, "UserSet",
                                                            request->getTypeName(), typeId,
                                                            request->getPageSize(), -1, 1, id);
                std::cout << "created data in sqlite database" << std::endl;
                idMap[std::pair<std::string, std::string>(request->getDatabaseName(), request->getSetName())] = id;
            }
         
            Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
            res = sendUsingMe->sendObject(response, errMsg);
            return make_pair(res, errMsg);

        }));

    forMe.registerHandler(
        DistributedStorageAddSet_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageAddSet>>([&](
            Handle<DistributedStorageAddSet> request, PDBCommunicatorPtr sendUsingMe) {
            const UseTemporaryAllocationBlock tempBlock{8 * 1024 * 1024};
            auto begin = std::chrono::high_resolution_clock::now();
            auto beforeCreateSet = begin;
            auto afterCreateSet = begin;

            PDB_COUT << "received DistributedStorageAddSet message" << std::endl;
            std::string errMsg;
            mutex lock;

            auto successfulNodes = std::vector<std::string>();
            auto failureNodes = std::vector<std::string>();
            auto nodesToBroadcast = std::vector<std::string>();

            std::string database = request->getDatabase();
            std::string set = request->getSetName();


            long lambdaId = -1;


            if (getFunctionality<CatalogClient>().setExists(database, set)) {
                std::cout << "Set " << database << ":" << set << " already exists " << std::endl;
            } else {
                PDB_COUT << "Set " << database << ":" << set << " does not exist" << std::endl;

                // JiaNote: comment out below line because searchForObjectTypeName doesn't work for
                // complex type like Vector<Handle<Foo>>
                // int16_t typeId =
                // getFunctionality<CatalogClient>().searchForObjectTypeName(request->getTypeName());
                int16_t typeId = VTableMap::getIDByName(VTableMap::getInternalTypeName(request->getTypeName()), false);
                if (typeId == 0) {
                    return make_pair(false, "Could not identify type=" + request->getTypeName());
                }

                beforeCreateSet = std::chrono::high_resolution_clock::now();

                if (!getFunctionality<CatalogClient>().createSet(request->getTypeName(), typeId, database, set, errMsg)) {
                    std::cout << "Could not register set, because: " << errMsg << std::endl;
                    Handle<SimpleRequestResult> response =
                        makeObject<SimpleRequestResult>(false, errMsg);
                    bool res = sendUsingMe->sendObject(response, errMsg);
                    return make_pair(res, errMsg);
                }
                afterCreateSet = std::chrono::high_resolution_clock::now();
            }
            std::vector<std::string> allNodes;
            const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
            for (int i = 0; i < nodes->size(); i++) {
                std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                std::string port = std::to_string((*nodes)[i]->getPort());
                allNodes.push_back(address + ":" + port);
            }
            nodesToBroadcast = allNodes;
            auto catalogGetNodesEnd = std::chrono::high_resolution_clock::now();
            std::shared_ptr<AbstractDataPlacementOptimizer> optimizer = nullptr;
            size_t pageSize = request->getPageSize();
            //to get ShuffleInfo object
            std::shared_ptr<ShuffleInfo> shuffleInfo = getFunctionality<QuerySchedulerServer>().getShuffleInfo();;
            if ((this->selfLearningOrNot == true)&&(request->getLambdaIdentifier() == nullptr)&&(trainingOrNot == false)) {
#ifdef APPLY_REINFORCEMENT_LEARNING
                optimizer = std::make_shared<DRLBasedDataPlacementOptimizerForLoadJob> (shuffleInfo->getNumHashPartitions(),
                                                                      shuffleInfo->getNumNodes(),
                                                                      request->getCreatedJobId(),
                                                                      request->getTypeName(),                                                                      
                                                                      getFunctionality<SelfLearningWrapperServer>(),
                                                                      make_shared<RLClient>(8333, "localhost", this->logger));
#else
                optimizer = std::make_shared<RuleBasedDataPlacementOptimizerForLoadJob> (shuffleInfo->getNumHashPartitions(),  
                                                                      request->getCreatedJobId(),
                                                                      request->getTypeName(),
                                                                      getFunctionality<SelfLearningWrapperServer>());
#endif
#ifdef OPTIMIZE_PAGE_SIZE
                
                pageSize = optimizer->getBestPageSize();
                if ((pageSize == 0) || (pageSize > request->getPageSize()))  {
                    pageSize = request->getPageSize();
                }
                if (pageSize < 1*1024*1024) {
                    pageSize = 1*1024*1024;
                } 
#endif
            }

            size_t desiredSize = request->getDesiredSize();
            if (desiredSize == 0) {
                if (optimizer != nullptr) {
                    desiredSize = optimizer->getEstimatedSize() / pageSize;
                }
            }
            if (desiredSize == 0) {
                desiredSize = 1000;
            }

            std::cout << "******************** desired size = " << desiredSize << "********************" << std::endl;
            Handle<StorageAddSet> storageCmd = makeObject<StorageAddSet>(request->getDatabase(),
                                                                         request->getSetName(),
                                                                         request->getTypeName(),
                                                                         pageSize,
                                                                         desiredSize,
                                                                         request->getMRUorNot(),
                                                                         request->getMRUorNot());
            std::cout << "Page size is determined to be " << pageSize << std::endl;
            

            //there are five cases:
            //case 1. distributedComputation is null, lambdaIdentifier is null, and doesn't use selfLearning;
            //case 2. distributedComputation is null, lambdaIdentifier is null, and use selfLearning;
            //case 3. distributedComputation is not null, lambdaIdentifier is null, use distributedComputation;
            //case 4. distributedComputation is null, lambdaIdentifier is not null, use lambdaIdentifier;
            //case 5. distributedComputation is not null, lambdaIdentifier is not null, use distributedComputation;

            Handle<Computation> myComputation = request->getDispatchComputation();
            Handle<LambdaIdentifier> myLambdaIdentifier = request->getLambdaIdentifier();
            if (myLambdaIdentifier == nullptr) {
                 if ((this->selfLearningOrNot == true) && (optimizer != nullptr)&&(this->trainingOrNot == false)){
                     std::cout << "to get the best lambda" << std::endl;
                     myLambdaIdentifier = optimizer->getBestLambda();
                 }
            }
            if (myComputation != nullptr) {
                 //to get dispatch computation from client (irrelevant with self-learning)
                 //to fetch the lambda object
                 //TODO
                 //to create a LambdaPolicy
                 //TODO
                 //to set the LambdaPolicy
                 //TODO
            } else if (myLambdaIdentifier != nullptr) {
                     //to fetch the lambda object
                     std::string jobName = myLambdaIdentifier->getJobName();
                     std::string computationName = myLambdaIdentifier->getComputationName();
                     std::string lambdaName = myLambdaIdentifier->getLambdaName();
                      std::cout << "my best lambda is " << jobName << ", " << computationName << ", " << lambdaName << std::endl;
                     //to get the computation object 
                     Handle<Computation> myComputation = getFunctionality<SelfLearningServer>().getComputation(
                       jobName, computationName);
                     //to get the lambda object
                     if (myComputation != nullptr) {
                         ComputationNode myNode(myComputation);
                         std::cout << "lambda name is " << lambdaName << std::endl;
                         GenericLambdaObjectPtr myLambda = myNode.getLambda(lambdaName); 

                         if (myLambda != nullptr) {   
                             int numNodes = shuffleInfo->getNumNodes();
                             int numPartitions = shuffleInfo->getNumHashPartitions();    
                             std::cout << "numNodes = " << numNodes << std::endl;
                             std::cout << "numPartitions = " << numPartitions << std::endl;
                             //to create a LambdaPolicy
                             PartitionPolicyPtr myLambdaPolicy = std::make_shared<LambdaPolicy>(numNodes, numPartitions, myLambda);
                             //to set the LambdaPolicy
                             std::cout << "to register policy" << std::endl;
#ifndef TEST_LACHESIS_OVERHEAD
                             getFunctionality<DispatcherServer>().registerSet(std::pair<std::string, std::string>(request->getSetName(), request->getDatabase()), myLambdaPolicy);
#endif
                         }
                     }
                     lambdaId = getFunctionality<SelfLearningServer>().getLambdaId(jobName, computationName, lambdaName);
                     std::cout << "the lambda id is " << lambdaId << std::endl;

            } else {
                 std::cout << "No Computation and Lambda for partitioning" << std::endl;
                 if (this->selfLearningOrNot == true) {
                 //to fetch the lambda object
                 //TODO
                 //to create a LambdaPolicy
                 //TODO
                 //to set the LambdaPolicy
                 //TODO
                 }
            }

#ifdef APPLY_REINFORCEMENT_LEARNING 
            bool isRepeatedLambda = false;
            if ((this->selfLearningOrNot == true) && (this->trainingOrNot == false)) {
                long latestLambdaId = getFunctionality<SelfLearningServer>().getLatestLambdaIdForLoadJob(request->getCreatedJobId());
                std::cout << "received lambdaId is " << lambdaId << std::endl;
                std::cout << "last lambdaId applied to the same data is " << latestLambdaId << std::endl;
                if (latestLambdaId == lambdaId) {
                    isRepeatedLambda = true;
                } else {
                    isRepeatedLambda = false;
                }
            }
#endif

            std::cout << "to broadcast StorageAddset" << std::endl; 
            getFunctionality<DistributedStorageManagerServer>()
                .broadcast<StorageAddSet, Object, SimpleRequestResult>(
                    storageCmd,
                    nullptr,
                    nodesToBroadcast,
                    generateAckHandler(successfulNodes, failureNodes, lock));
            std::cout << "broadcasted StorageAddSet" << std::endl;
            auto storageAddSetEnd = std::chrono::high_resolution_clock::now();


            bool res = true;
            if (failureNodes.size() > 0) {
                res = false;
            } else {
                // update stats
                StatisticsPtr stats = getFunctionality<QuerySchedulerServer>().getStats();
                if (stats == nullptr) {
                    getFunctionality<QuerySchedulerServer>().collectStats();
                    stats = getFunctionality<QuerySchedulerServer>().getStats();
                }
                stats->setNumPages(request->getDatabase(), request->getSetName(), 0);
                stats->setNumBytes(request->getDatabase(), request->getSetName(), 0);
            }

            if (this->selfLearningOrNot == true) {
                long id;
                std::string createdJobId = request->getCreatedJobId();
                if (createdJobId == "") {
                    createdJobId = this->getNextClientId();
                }
                std::cout << "createdJobId is " << createdJobId << std::endl;
                int typeId = VTableMap::getIDByName(request->getTypeName());
                std::cout << "typeId is " << typeId << std::endl;
                std::cout << "create data in sqlite for " << request->getDatabase() << ":" << request->getSetName() << std::endl;
                getFunctionality<SelfLearningServer>().createData(request->getDatabase(), request->getSetName(),
                                                            createdJobId, "UserSet", 
                                                            request->getTypeName(), typeId, 
                                                            request->getPageSize(), lambdaId, 1, id);
                std::cout << "created data in sqlite" << std::endl;
                idMap[std::pair<std::string, std::string>(request->getDatabase(), request->getSetName())] = id;
            }
            auto catalogAddSetEnd = std::chrono::high_resolution_clock::now();

            PDB_COUT << "Time Duration for catalog create set Metadata:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(afterCreateSet -
                                                                                 beforeCreateSet)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << "Time Duration for catalog getting nodes:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(
                            catalogGetNodesEnd - afterCreateSet)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << "Time Duration for storage adding set:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(storageAddSetEnd -
                                                                                 catalogGetNodesEnd)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << "Time Duration for catalog adding addNodeToSet metadata:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(catalogAddSetEnd -
                                                                                 storageAddSetEnd)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << std::endl;

#ifdef APPLY_REINFORCEMENT_LEARNING 
            if ((this->selfLearningOrNot == true)&&(this->trainingOrNot == false)) {
                if ((res == true) && (isRepeatedLambda == true)) {
                    res = false;
                    errMsg = "RepeatedLambda";
                }
            }
#endif

            Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
            res = sendUsingMe->sendObject(response, errMsg);
            return make_pair(res, errMsg);
        }));


    forMe.registerHandler(
        DistributedStorageAddSetWithPartition_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageAddSetWithPartition>>([&](
            Handle<DistributedStorageAddSetWithPartition> request, PDBCommunicatorPtr sendUsingMe) {
            const UseTemporaryAllocationBlock tempBlock{128 * 1024 * 1024};
            auto begin = std::chrono::high_resolution_clock::now();
            auto beforeCreateSet = begin;
            auto afterCreateSet = begin;
            bool success;
            std::string errMsg;
            Handle<Vector<Handle<Computation>>> myComputations =
                    sendUsingMe->getNextObject<Vector<Handle<Computation>>>(success, errMsg);
            PDB_COUT << "received DistributedStorageAddSetWithPartition message" << std::endl;
            mutex lock;

            auto successfulNodes = std::vector<std::string>();
            auto failureNodes = std::vector<std::string>();
            auto nodesToBroadcast = std::vector<std::string>();

            std::string database = request->getDatabase();
            std::string set = request->getSetName();
            std::string jobName = request->getJobName();
            std::cout <<"create database="<<database<<", set="<<set<<",using partitioning computation extracted from job=" << jobName; 

            long lambdaId = -1;
            long lambdaId1 = -1;

            if (getFunctionality<CatalogClient>().setExists(database, set)) {
                std::cout << "Set " << database << ":" << set << " already exists " << std::endl;
            } else {
                PDB_COUT << "Set " << database << ":" << set << " does not exist" << std::endl;

                // JiaNote: comment out below line because searchForObjectTypeName doesn't work for
                // complex type like Vector<Handle<Foo>>
                // int16_t typeId =
                // getFunctionality<CatalogClient>().searchForObjectTypeName(request->getTypeName());
                int16_t typeId = VTableMap::getIDByName(VTableMap::getInternalTypeName(request->getTypeName()), false);
                if (typeId == 0) {
                    return make_pair(false, "Could not identify type=" + request->getTypeName());
                }

                beforeCreateSet = std::chrono::high_resolution_clock::now();

                if (!getFunctionality<CatalogClient>().createSet(request->getTypeName(), typeId, database, set, errMsg)) {
                    std::cout << "Could not register set, because: " << errMsg << std::endl;
                    Handle<SimpleRequestResult> response =
                        makeObject<SimpleRequestResult>(false, errMsg);
                    bool res = sendUsingMe->sendObject(response, errMsg);
                    return make_pair(res, errMsg);
                }
                afterCreateSet = std::chrono::high_resolution_clock::now();
            }
            std::vector<std::string> allNodes;
            const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
            for (int i = 0; i < nodes->size(); i++) {
                std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                std::string port = std::to_string((*nodes)[i]->getPort());
                allNodes.push_back(address + ":" + port);
            }
            nodesToBroadcast = allNodes;
            auto catalogGetNodesEnd = std::chrono::high_resolution_clock::now();
            std::shared_ptr<AbstractDataPlacementOptimizer> optimizer = nullptr;
            size_t pageSize = request->getPageSize();
            //to get ShuffleInfo object
            std::shared_ptr<ShuffleInfo> shuffleInfo = getFunctionality<QuerySchedulerServer>().getShuffleInfo();;
            size_t desiredSize = request->getDesiredSize();
            if (desiredSize == 0) {
                desiredSize = 1000;
            }

            std::cout << "******************** desired size = " << desiredSize << "********************" << std::endl;
            Handle<StorageAddSet> storageCmd = makeObject<StorageAddSet>(request->getDatabase(),
                                                                         request->getSetName(),
                                                                         request->getTypeName(),
                                                                         pageSize,
                                                                         desiredSize,
                                                                         request->getMRUorNot(),
                                                                         request->getMRUorNot());
            std::cout << "Page size is determined to be " << pageSize << std::endl;


            std::pair<std::string, std::string> source;
            source.first = request->getDatabase();
            source.second = request->getSetName();

            int numNodes = shuffleInfo->getNumNodes();
            int numPartitions = shuffleInfo->getNumHashPartitions();
            std::cout << "numNodes = " << numNodes << std::endl;
            std::cout << "numPartitions = " << numPartitions << std::endl;
            //to create an IRPolicy
            PartitionPolicyPtr myIRPolicy = std::make_shared<IRPolicy>(numNodes, numPartitions, myComputations, source);
            //to set the IRPolicy
            std::cout << "to register policy" << std::endl;
            getFunctionality<DispatcherServer>().registerSet(std::pair<std::string, std::string>(request->getSetName(), request->getDatabase()), myIRPolicy);
            lambdaId = getFunctionality<SelfLearningServer>().getLambdaId("reddit-a", "JoinComp_3", "attAccess_0");
            lambdaId1 = getFunctionality<SelfLearningServer>().getLambdaId("reddit-s", "JoinComp_3", "attAccess_0");
            std::cout << "the lambda id is " << lambdaId << std::endl;
            std::cout << "the lambda id1 is " << lambdaId1 << std::endl;

            std::cout << "to broadcast StorageAddset" << std::endl;
            getFunctionality<DistributedStorageManagerServer>()
                .broadcast<StorageAddSet, Object, SimpleRequestResult>(
                    storageCmd,
                    nullptr,
                    nodesToBroadcast,
                    generateAckHandler(successfulNodes, failureNodes, lock));
            std::cout << "broadcasted StorageAddSet" << std::endl;
            auto storageAddSetEnd = std::chrono::high_resolution_clock::now();


            bool res = true;
            if (failureNodes.size() > 0) {
                res = false;
            } else {
                // update stats
                StatisticsPtr stats = getFunctionality<QuerySchedulerServer>().getStats();
                if (stats == nullptr) {
                    getFunctionality<QuerySchedulerServer>().collectStats();
                    stats = getFunctionality<QuerySchedulerServer>().getStats();
                }
                stats->setNumPages(request->getDatabase(), request->getSetName(), 0);
                stats->setNumBytes(request->getDatabase(), request->getSetName(), 0);
            }

            if (this->selfLearningOrNot == true) {
                long id;
                std::string createdJobId = request->getCreatedJobId();
                if (createdJobId == "") {
                    createdJobId = this->getNextClientId();
                }
                std::cout << "createdJobId is " << createdJobId << std::endl;
                int typeId = VTableMap::getIDByName(request->getTypeName());
                std::cout << "typeId is " << typeId << std::endl;
                std::cout << "create data in sqlite for " << request->getDatabase() << ":" << request->getSetName() << std::endl;
                getFunctionality<SelfLearningServer>().createData(request->getDatabase(), request->getSetName(),
                                                            createdJobId, "UserSet",
                                                            request->getTypeName(), typeId,
                                                            request->getPageSize(), lambdaId, 1, id, lambdaId1);
                std::cout << "created data in sqlite" << std::endl;
                idMap[std::pair<std::string, std::string>(request->getDatabase(), request->getSetName())] = id;
            }
            auto catalogAddSetEnd = std::chrono::high_resolution_clock::now();

            PDB_COUT << "Time Duration for catalog create set Metadata:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(afterCreateSet -
                                                                                 beforeCreateSet)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << "Time Duration for catalog getting nodes:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(
                            catalogGetNodesEnd - afterCreateSet)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << "Time Duration for storage adding set:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(storageAddSetEnd -
                                                                                 catalogGetNodesEnd)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << "Time Duration for catalog adding addNodeToSet metadata:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(catalogAddSetEnd -
                                                                                 storageAddSetEnd)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << std::endl;


            Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
            res = sendUsingMe->sendObject(response, errMsg);
            return make_pair(res, errMsg);
        }));



    forMe.registerHandler(
        DistributedStorageRemoveDatabase_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageRemoveDatabase>>([&](
            Handle<DistributedStorageRemoveDatabase> request, PDBCommunicatorPtr sendUsingMe) {
            const UseTemporaryAllocationBlock tempBlock{1 * 1024 * 1024};
            std::string errMsg;
            mutex lock;
            std::vector<std::string> successfulNodes = std::vector<std::string>();
            std::vector<std::string> failureNodes = std::vector<std::string>();

            std::string database = request->getDatabase();

            if (!getFunctionality<CatalogClient>().databaseExists(
                    database)) {
                errMsg = "Cannot delete database, database " + database + " does not exist\n";
                Handle<SimpleRequestResult> response =
                    makeObject<SimpleRequestResult>(false, errMsg);
                bool res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }

            auto nodesToBroadcastTo = std::vector<std::string>();

            std::vector<std::string> allNodes;
            const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
            for (int i = 0; i < nodes->size(); i++) {
                std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                std::string port = std::to_string((*nodes)[i]->getPort());
                allNodes.push_back(address + ":" + port);
            }
            nodesToBroadcastTo = allNodes;
            Handle<StorageRemoveDatabase> storageCmd = makeObject<StorageRemoveDatabase>(database);
            getFunctionality<DistributedStorageManagerServer>()
                .broadcast<StorageRemoveDatabase, Object, SimpleRequestResult>(
                    storageCmd,
                    nullptr,
                    nodesToBroadcastTo,
                    generateAckHandler(successfulNodes, failureNodes, lock));

            if (failureNodes.size() == 0) {
                PDB_COUT << "Successfully deleted database on " << successfulNodes.size()
                         << " nodes" << std::endl;
            } else {
                errMsg = "Failed to delete database on " + std::to_string(failureNodes.size()) +
                    " nodes. Skipping registering with catalog";
                std::cout << errMsg;
                Handle<SimpleRequestResult> response =
                    makeObject<SimpleRequestResult>(false, errMsg);
                bool res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }

            if (!getFunctionality<CatalogClient>().deleteDatabase(database, errMsg)) {
                std::cout << "Could not delete database, because: " << errMsg << std::endl;
                Handle<SimpleRequestResult> response =
                    makeObject<SimpleRequestResult>(false, errMsg);
                bool res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }

            bool res = true;

            Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
            res = sendUsingMe->sendObject(response, errMsg);
            return make_pair(res, errMsg);
        }));

    forMe.registerHandler(
        DistributedStorageRemoveTempSet_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageRemoveTempSet>>(
            [&](Handle<DistributedStorageRemoveTempSet> request, PDBCommunicatorPtr sendUsingMe) {
                const UseTemporaryAllocationBlock tempBlock{1 * 1024 * 1024};
                auto begin = std::chrono::high_resolution_clock::now();

                std::string errMsg;
                mutex lock;
                auto successfulNodes = std::vector<std::string>();
                auto failureNodes = std::vector<std::string>();
                auto nodesToBroadcast = std::vector<std::string>();

                std::string database = request->getDatabase();
                std::string set = request->getSetName();
                std::string fullSetName = database + "." + set;
                std::vector<std::string> allNodes;
                const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
                for (int i = 0; i < nodes->size(); i++) {
                    std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                    std::string port = std::to_string((*nodes)[i]->getPort());
                    allNodes.push_back(address + ":" + port);
                }
                nodesToBroadcast = allNodes;

                PDB_COUT << "to broadcast StorageRemoveTempSet" << std::endl;
                Handle<StorageRemoveUserSet> storageCmd = makeObject<StorageRemoveUserSet>(
                    request->getDatabase(), request->getSetName(), request->getTypeName());
                getFunctionality<DistributedStorageManagerServer>()
                    .broadcast<StorageRemoveUserSet, Object, SimpleRequestResult>(
                        storageCmd,
                        nullptr,
                        nodesToBroadcast,
                        generateAckHandler(successfulNodes, failureNodes, lock));

                auto storageRemoveSetEnd = std::chrono::high_resolution_clock::now();

                PDB_COUT << "Time Duration for storage removing set:\t "
                         << std::chrono::duration_cast<std::chrono::duration<float>>(
                                storageRemoveSetEnd - begin)
                                .count()
                         << " secs." << std::endl;

                bool res = true;
                if (failureNodes.size() > 0) {
                    res = false;
                    errMsg = "";
                    for (int i = 0; i < failureNodes.size(); i++) {
                        errMsg += failureNodes[i] + std::string(";");
                    }
                }
                // update stats
                StatisticsPtr stats = getFunctionality<QuerySchedulerServer>().getStats();
                if (stats == nullptr) {
                    getFunctionality<QuerySchedulerServer>().collectStats();
                    stats = getFunctionality<QuerySchedulerServer>().getStats();
                }
                stats->removeSet(request->getDatabase(), request->getSetName());
                if (selfLearningOrNot == true) {
                    long id = getIdForData(request->getDatabase(), request->getSetName());
                    getFunctionality<SelfLearningServer>().updateDataForRemoval(id);
                    removeData(request->getDatabase(), request->getSetName());
            
                }
                Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
                res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }));


    forMe.registerHandler(
        DistributedStorageRemoveSet_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageRemoveSet>>([&](
            Handle<DistributedStorageRemoveSet> request, PDBCommunicatorPtr sendUsingMe) {

            const UseTemporaryAllocationBlock tempBlock{1 * 1024 * 1024};
            auto begin = std::chrono::high_resolution_clock::now();

            std::string errMsg;
            mutex lock;
            auto successfulNodes = std::vector<std::string>();
            auto failureNodes = std::vector<std::string>();
            auto nodesToBroadcast = std::vector<std::string>();

            std::string databaseName = request->getDatabase();
            std::string setName = request->getSetName();
            std::string fullSetName = databaseName + "." + setName;
            std::string typeName;

            std::string error;
            auto set = getFunctionality<CatalogClient>().getSet(databaseName, setName, error);

            if (set == nullptr) {
                std::cout << "Cannot remove set, Set " << fullSetName << " does not exist "
                          << std::endl;
                Handle<SimpleRequestResult> response =
                    makeObject<SimpleRequestResult>(false, errMsg);
                bool res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }

            // this should basically never happen
            assert(set->type != nullptr);

            // grab the type associated with the set
            auto type = getFunctionality<CatalogClient>().getType(*set->type, errMsg);

            // check if the type exists
            if(type == nullptr) {
              std::cout << "Remove set, can not find the type with the id : " << *set->type << "\n";
              Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(false, errMsg);
              bool res = sendUsingMe->sendObject(response, errMsg);
              return make_pair(res, errMsg);
            }

            // set the type name
            typeName = type->name;


#ifndef USING_ALL_NODES
            if (!getFunctionality<DistributedStorageManagerServer>().findNodesContainingSet(
                    database, set, nodesToBroadcast, errMsg)) {
                std::cout << "Could not find nodes to broadcast set to: " << errMsg << std::endl;
                Handle<SimpleRequestResult> response =
                    makeObject<SimpleRequestResult>(false, errMsg);
                bool res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }
#else
            std::vector<std::string> allNodes;
            const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
            for (int i = 0; i < nodes->size(); i++) {
                std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                std::string port = std::to_string((*nodes)[i]->getPort());
                allNodes.push_back(address + ":" + port);
            }
            nodesToBroadcast = allNodes;
#endif

            auto catalogGetNodesEnd = std::chrono::high_resolution_clock::now();
            PDB_COUT << "to broadcast StorageRemoveUserSet" << std::endl;
            Handle<StorageRemoveUserSet> storageCmd = makeObject<StorageRemoveUserSet>(
                request->getDatabase(), request->getSetName(), typeName);
            getFunctionality<DistributedStorageManagerServer>()
                .broadcast<StorageRemoveUserSet, Object, SimpleRequestResult>(
                    storageCmd,
                    nullptr,
                    nodesToBroadcast,
                    generateAckHandler(successfulNodes, failureNodes, lock));

            if (failureNodes.size() == 0) {
                PDB_COUT << "Successfully deleted set on " << successfulNodes.size() << " nodes"
                         << std::endl;
            } else {
                errMsg = "Failed to delete set on " + std::to_string(failureNodes.size()) +
                    " nodes. Skipping registering with catalog";
                std::cout << errMsg << std::endl;
                Handle<SimpleRequestResult> response =
                    makeObject<SimpleRequestResult>(false, errMsg);
                bool res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);
            }

            auto storageRemoveSetEnd = std::chrono::high_resolution_clock::now();

            // update stats
            StatisticsPtr stats = getFunctionality<QuerySchedulerServer>().getStats();
            if (stats == nullptr) {
                getFunctionality<QuerySchedulerServer>().collectStats();
                stats = getFunctionality<QuerySchedulerServer>().getStats();
            }
            stats->removeSet(request->getDatabase(), request->getSetName());
            if (selfLearningOrNot == true) {
                long id = getIdForData(request->getDatabase(), request->getSetName());
                getFunctionality<SelfLearningServer>().updateDataForRemoval(id);
                removeData(request->getDatabase(), request->getSetName());

            }
            //to deregister the partition policy for this set
#ifndef TEST_LACHESIS_OVERHEAD
            getFunctionality<DispatcherServer>().deregisterSet(std::pair<std::string, std::string>(
                     request->getSetName(), request->getDatabase()));
#endif

            if (failureNodes.size() == 0) {
                // If all the nodes succeeded in removing the set then we can simply delete the set
                // from the
                // catalog
                PDB_COUT << "Succeeded in deleting set " << fullSetName << " on all nodes"
                         << std::endl;
                if (!getFunctionality<CatalogClient>().deleteSet(databaseName, setName, errMsg)) {
                    std::cout << "Could not delete set, because: " << errMsg << std::endl;
                    Handle<SimpleRequestResult> response =
                        makeObject<SimpleRequestResult>(false, errMsg);
                    bool res = sendUsingMe->sendObject(response, errMsg);
                    return make_pair(res, errMsg);
                }
            } else {
                // If some nodes failed in removing the set remove these nodes from the set maps in
                // the catalog
                // so that future calls to this function will only attempt to affect the unmodified
                // storage nodes
                errMsg = "Nodes failed to remove set " + fullSetName + ": ";

                bool res = false;
                Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
                sendUsingMe->sendObject(response, errMsg);
                return make_pair(false, errMsg);
            }

            auto catalogRemoveSetEnd = std::chrono::high_resolution_clock::now();
            bool res = true;
            Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
            res = sendUsingMe->sendObject(response, errMsg);


            PDB_COUT << "Time Duration for catalog get nodes info:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(
                            catalogGetNodesEnd - begin)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << "Time Duration for storage removing set:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(
                            storageRemoveSetEnd - catalogGetNodesEnd)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << "Time Duration for catalog removing set:\t "
                     << std::chrono::duration_cast<std::chrono::duration<float>>(
                            catalogRemoveSetEnd - storageRemoveSetEnd)
                            .count()
                     << " secs." << std::endl;
            PDB_COUT << std::endl;
            return make_pair(res, errMsg);

        }));

    // JiaNote: Below handler is to process DistributedStorageCleanup message, this handler is to
   // write back records on all slaves
    forMe.registerHandler(
        DistributedStorageCleanup_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageCleanup>>(

            [&](Handle<DistributedStorageCleanup> request, PDBCommunicatorPtr sendUsingMe) {
                const UseTemporaryAllocationBlock tempBlock{1 * 1024 * 1024};
                std::cout << "received DistributedStorageCleanup" << std::endl;
                std::string errMsg;
                mutex lock;
                auto successfulNodes = std::vector<std::string>();
                auto failureNodes = std::vector<std::string>();

                std::vector<std::string> allNodes;
                std::cout << "to wait for all requests get processed" << std::endl;
                getFunctionality<DispatcherServer>().waitAllRequestsProcessed();
                std::cout << "All data requests have been served" << std::endl;
                const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
                for (int i = 0; i < nodes->size(); i++) {
                    std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                    std::string port = std::to_string((*nodes)[i]->getPort());
                    allNodes.push_back(address + ":" + port);
                }

                Handle<StorageCleanup> storageCmd = makeObject<StorageCleanup>();

                getFunctionality<DistributedStorageManagerServer>()
                    .broadcast<StorageCleanup, Object, SimpleRequestResult>(
                        storageCmd,
                        nullptr,
                        allNodes,
                        generateAckHandler(successfulNodes, failureNodes, lock));

                bool res = true;
                if (failureNodes.size() > 0) {
                    res = false;
                    errMsg = "";
                    for (int i = 0; i < failureNodes.size(); i++) {
                        errMsg += failureNodes[i] + std::string(";");
                    }
                }

                Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
                res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);


            }

            ));

    // JiaNote: Below handler is to process DistributedStorageExportSet message, this handler is to
    // write back records on all slaves
    forMe.registerHandler(
        DistributedStorageExportSet_TYPEID,
        make_shared<SimpleRequestHandler<DistributedStorageExportSet>>(

            [&](Handle<DistributedStorageExportSet> request, PDBCommunicatorPtr sendUsingMe) {
                const UseTemporaryAllocationBlock tempBlock{1 * 1024 * 1024};
                PDB_COUT << "received DistributedStorageExportSet" << std::endl;
                std::string errMsg;
                mutex lock;
                auto successfulNodes = std::vector<std::string>();
                auto failureNodes = std::vector<std::string>();

                std::vector<std::string> allNodes;
                const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
                for (int i = 0; i < nodes->size(); i++) {
                    std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                    std::string port = std::to_string((*nodes)[i]->getPort());
                    allNodes.push_back(address + ":" + port);
                }

                Handle<StorageExportSet> storageCmd =
                    makeObject<StorageExportSet>(request->getDbName(),
                                                 request->getSetName(),
                                                 request->getOutputFilePath(),
                                                 request->getFormat());

                getFunctionality<DistributedStorageManagerServer>()
                    .broadcast<StorageExportSet, Object, SimpleRequestResult>(
                        storageCmd,
                        nullptr,
                        allNodes,
                        generateAckHandler(successfulNodes, failureNodes, lock));

                bool res = true;
                if (failureNodes.size() > 0) {
                    res = false;
                    errMsg = "";
                    for (int i = 0; i < failureNodes.size(); i++) {
                        errMsg += failureNodes[i] + std::string(";");
                    }
                }
                Handle<SimpleRequestResult> response = makeObject<SimpleRequestResult>(res, errMsg);
                res = sendUsingMe->sendObject(response, errMsg);
                return make_pair(res, errMsg);


            }

            ));


    // JiaNote: Below handler is to process SetScan message
    forMe.registerHandler(
        SetScan_TYPEID,
        make_shared<SimpleRequestHandler<SetScan>>([&](Handle<SetScan> request,
                                                       PDBCommunicatorPtr sendUsingMe) {
            const UseTemporaryAllocationBlock tempBlock{8 * 1024 * 1024};
            std::string errMsg;
            bool success;
            std::string dbName = request->getDatabase();
            std::string setName = request->getSetName();
            PDB_COUT << "DistributedStorageManager received SetScan message: dbName =" << dbName
                     << ", setName =" << setName << std::endl;

            // to check whether set exists
            /*
            std :: string fullSetName = dbName + "." + setName;
            std :: string value;
            int catalogType = PDBCatalogMsgType::CatalogPDBSet;
            if (getFunctionality<CatalogServer>().getCatalog()->keyIsFound(catalogType, fullSetName,
            value)) {
                PDB_COUT << "Set " << fullSetName << " exists" << std :: endl;
            } else {
                errMsg = "Error in handling SetScan message: Set does not exist";
                std :: cout << errMsg << std :: endl;
                return make_pair(false, errMsg);
            }
            */
            // to get all nodes having data for this set
            std::vector<std::string> nodesToBroadcast;
#ifndef USING_ALL_NODES
            if (!getFunctionality<DistributedStorageManagerServer>().findNodesContainingSet(
                    dbName, setName, nodesToBroadcast, errMsg)) {
                errMsg = "Error in handling SetScan message: Could not find nodes for this set";
                std::cout << errMsg << std::endl;
                return make_pair(false, errMsg);
            }
#else
            std::vector<std::string> allNodes;
            const auto nodes = getFunctionality<ResourceManagerServer>().getAllNodes();
            for (int i = 0; i < nodes->size(); i++) {
                std::string address = static_cast<std::string>((*nodes)[i]->getAddress());
                std::string port = std::to_string((*nodes)[i]->getPort());
                allNodes.push_back(address + ":" + port);
            }
            nodesToBroadcast = allNodes;

#endif
            PDB_COUT << "num nodes for this set" << nodesToBroadcast.size() << std::endl;

            // to send SetScan message to slave servers iteratively

            char* curPage = nullptr;
            Handle<KeepGoing> temp;
            bool keepGoingSent = false;
            for (int i = 0; i < nodesToBroadcast.size(); i++) {
                std::string serverName = nodesToBroadcast[i];
                int port;
                std::string address;
                size_t pos = serverName.find(":");
                if (pos != string::npos) {
                    port = stoi(serverName.substr(pos + 1, serverName.size()));
                    address = serverName.substr(0, pos);
                } else {
                    if (conf != nullptr) {
                        port = conf->getPort();
                    } else {
                        port = 8108;
                    }
                    address = serverName;
                }


                PDB_COUT << "to collect data from the " << i
                         << "-th server with address=" << address << " and port=" << port
                         << std::endl;
                Handle<SetScan> newRequest =
                    makeObject<SetScan>(request->getDatabase(), request->getSetName());

                PDB_COUT << "to connect to the remote node" << std::endl;
                PDBCommunicatorPtr communicator = std::make_shared<PDBCommunicator>();

                PDB_COUT << "port:" << port << std::endl;
                PDB_COUT << "ip address:" << address << std::endl;

                if (communicator->connectToInternetServer(logger, port, address, errMsg)) {
                    success = false;
                    std::cout << errMsg << std::endl;
                    break;
                }

                if (!communicator->sendObject(newRequest, errMsg)) {
                    success = false;
                    std::cout << errMsg << std::endl;
                    break;
                }
                std::cout << "sent SetScan object to " << address << std::endl;
                while (true) {
                    if (curPage != nullptr) {
                        free(curPage);
                        curPage = nullptr;
                        if (keepGoingSent == false) {
                            if (sendUsingMe->getObjectTypeID() != DoneWithResult_TYPEID) {
                                Handle<KeepGoing> temp =
                                    sendUsingMe->getNextObject<KeepGoing>(success, errMsg);
                                if (!success) {
                                    // std :: cout << "DistributedStorageMangerServer: Problem
                                    // getting keep going from client: "<< errMsg << std :: endl;
                                    communicator = nullptr;
                                    break;
                                }
                                // std :: cout << "got keep going" << std :: endl;
                                if (!communicator->sendObject(temp, errMsg)) {
                                    std::cout << "Problem forwarding keep going: " << errMsg
                                              << std::endl;
                                    communicator = nullptr;
                                    break;
                                }
                                // std :: cout << "sent keep going" << std :: endl;
                                keepGoingSent = true;
                            } else {
                                Handle<DoneWithResult> doneMsg =
                                    sendUsingMe->getNextObject<DoneWithResult>(success, errMsg);
                                if (!success) {
                                    std::cout
                                        << "Problem getting done message from client: " << errMsg
                                        << std::endl;
                                    communicator = nullptr;
                                    return std::make_pair(false, errMsg);
                                }
                                // std :: cout << "got done from this client!" <<  std :: endl;
                                if (!communicator->sendObject(doneMsg, errMsg)) {
                                    std::cout << "Problem forwarding done message: " << errMsg
                                              << std::endl;
                                    communicator = nullptr;
                                    return std::make_pair(false, errMsg);
                                }
                                // std :: cout << "sent done message!" << std :: endl;
                                return std::make_pair(true, errMsg);
                            }
                        }
                    }
                    size_t objSize = communicator->getSizeOfNextObject();
                    // std :: cout << "Distributed storage to receive size " << objSize << std ::
                    // endl;
                    if (communicator->getObjectTypeID() == DoneWithResult_TYPEID) {
                        PDB_COUT << "got done from this slave!" << std::endl;
                        communicator = nullptr;
                        break;
                    }
                    curPage = (char*)malloc(objSize);
                    if (!communicator->receiveBytes(curPage, errMsg)) {
                        std::cout << "Problem getting data from slave: " << errMsg << std::endl;
                        communicator = nullptr;
                        break;
                    }
                    if (!sendUsingMe->sendBytes(curPage, objSize, errMsg)) {
                        std::cout << "Problem forwarding data to client: " << errMsg << std::endl;
                        communicator = nullptr;
                        break;
                    }
                    // std :: cout << "sent data to client!" << std :: endl;
                    keepGoingSent = false;
                }
            }
            Handle<DoneWithResult> doneWithResult = makeObject<DoneWithResult>();
            if (!sendUsingMe->sendObject(doneWithResult, errMsg)) {
                std::cout << "Problem sending done message to client: " << errMsg << std::endl;
                return std::make_pair(false, "could not send done message: " + errMsg);
            }
            PDB_COUT << "sent done message to client!" << std::endl;
            return std::make_pair(true, errMsg);

        }));
}

std::function<void(Handle<SimpleRequestResult>, std::string)>
DistributedStorageManagerServer::generateAckHandler(std::vector<std::string>& success,
                                                    std::vector<std::string>& failures,
                                                    mutex& lock) {
    return [&](Handle<SimpleRequestResult> response, std::string server) {
        lock.lock();

        // TODO: Better error handling

        if (!response->getRes().first) {
            PDB_COUT << "BROADCAST CALLBACK FAIL: " << server << ": " << response->getRes().first
                     << " : " << response->getRes().second << std::endl;
            failures.push_back(server);
        } else {
            PDB_COUT << "BROADCAST CALLBACK SUCCESS: " << server << ": " << response->getRes().first
                     << " : " << response->getRes().second << std::endl;
            success.push_back(server);
        }
        lock.unlock();
    };
}


}

#endif
