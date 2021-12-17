#ifndef OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERCLIENT_CC
#define OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERCLIENT_CC

#include "DistributedStorageManagerClient.h"

#include "SimpleRequest.h"
#include "DistributedStorageAddDatabase.h"
#include "DistributedStorageAddSet.h"
#include "DistributedStorageAddSetWithPartition.h"
#include "DistributedStorageAddTempSet.h"
#include "DistributedStorageRemoveDatabase.h"
#include "DistributedStorageRemoveSet.h"
#include "DistributedStorageRemoveHashSet.h"
#include "DistributedStorageRemoveTempSet.h"
#include "DistributedStorageExportSet.h"
#include "DistributedStorageClearSet.h"
#include "DistributedStorageCleanup.h"
#include "DistributedStorageAddSharedPage.h"

namespace pdb {

DistributedStorageManagerClient::DistributedStorageManagerClient() {}

DistributedStorageManagerClient::DistributedStorageManagerClient(int portIn,
                                                                 std::string addressIn,
                                                                 PDBLoggerPtr myLoggerIn)
    : port(portIn), address(addressIn), logger(myLoggerIn) {
    // no-op
}

DistributedStorageManagerClient::~DistributedStorageManagerClient() {
    // no-op
}

void DistributedStorageManagerClient::registerHandlers(PDBServer& forMe) {
    // no-op
}

bool DistributedStorageManagerClient::createDatabase(const std::string& databaseName,
                                                     std::string& errMsg) {
    return simpleRequest<DistributedStorageAddDatabase, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not add database to distributed storage manager", errMsg),
        databaseName);
}

bool DistributedStorageManagerClient::createSet(const std::string& databaseName,
                                                const std::string& setName,
                                                const std::string& typeName,
                                                std::string& errMsg,
                                                size_t pageSize,
                                                const std::string& createdJobId,
                                                Handle<Computation> dispatchComputation,
                                                Handle<LambdaIdentifier> lambdaIdentifier,
                                                size_t desiredSize,
                                                bool isMRU,
	       bool isSharedTensorBlockSet	) {
    std::cout << "to create set for " << databaseName << ":" << setName << std::endl;
    if (lambdaIdentifier != nullptr) {

         std::cout << "jobName is " << lambdaIdentifier->getJobName() << std::endl;
         std::cout << "computationName is " << lambdaIdentifier->getComputationName() << std::endl;
         std::cout << "lambdaName is " << lambdaIdentifier->getLambdaName() << std::endl;
    }
    return simpleRequest<DistributedStorageAddSet, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not add set to distributed storage manager:", errMsg),
        databaseName,
        setName,
        typeName,
        pageSize,
        createdJobId,
        dispatchComputation,
        lambdaIdentifier,
        desiredSize,
        isMRU,
	isSharedTensorBlockSet
        );
}


bool DistributedStorageManagerClient::createSet(const std::string& databaseName,
                                                const std::string& setName,
                                                const std::string& typeName,
                                                std::string& errMsg,
                                                size_t pageSize,
                                                const std::string& createdJobId,
                                                Handle<Vector<Handle<Computation>>> dispatchComputations,
                                                std::string jobName,
                                                std::string jobName1,
                                                std::string jobName2,
                                                std::string computationName1,
                                                std::string computationName2,
                                                std::string lambdaName1,
                                                std::string lambdaName2,
                                                size_t desiredSize,
                                                bool isMRU,
	                                        bool isSharedTensorBlockSet	) {
    std::cout << "to create set for " << databaseName << ":" << setName << std::endl;
    std::cout << "jobName is " << jobName << std::endl;
    Handle<DistributedStorageAddSetWithPartition> request = makeObject<DistributedStorageAddSetWithPartition> (databaseName,
    setName, typeName, pageSize, createdJobId, dispatchComputations, jobName, jobName1, jobName2, computationName1, computationName2, lambdaName1, lambdaName2, desiredSize, isMRU, isSharedTensorBlockSet);
    return simpleDoubleRequest<DistributedStorageAddSetWithPartition, Vector<Handle<Computation>>, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        [&](Handle<SimpleRequestResult> result) {
            if (result != nullptr) {
                if (!result->getRes().first) {
                    errMsg = "Error in create set: " + result->getRes().second;
                    logger->error(errMsg);
                    return false;
                }
                return true;
            }
            errMsg = "Error creating set with IR partitioning";
            return false;
         },
        request,
        dispatchComputations);
    }


bool DistributedStorageManagerClient::createTempSet(const std::string& databaseName,
                                                    const std::string& setName,
                                                    const std::string& typeName,
                                                    std::string& errMsg,
                                                    size_t pageSize,
                                                    const std::string& createdJobId,
                                                    Handle<Computation> dispatchComputation,
                                                    Handle<LambdaIdentifier> lambdaIdentifier,
                                                    size_t desiredSize,
                                                    bool isMRU ) {
    return simpleRequest<DistributedStorageAddTempSet, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,

        generateResponseHandler("Could not add temp set to distributed storage manager:", errMsg),
        databaseName,
        setName,
        typeName,
        pageSize,
        createdJobId,
        dispatchComputation,
        lambdaIdentifier,
        desiredSize,
        isMRU
        );
}

bool DistributedStorageManagerClient::removeDatabase(const std::string& databaseName,
                                                     std::string& errMsg) {
    return simpleRequest<DistributedStorageRemoveDatabase, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not remove database from distributed storage manager",
                                errMsg),
        databaseName);
}

bool DistributedStorageManagerClient::removeSet(const std::string& databaseName,
                                                const std::string& setName,
                                                std::string& errMsg) {
    return simpleRequest<DistributedStorageRemoveSet, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not remove set to distributed storage manager", errMsg),
        databaseName,
        setName);
}




bool DistributedStorageManagerClient::removeTempSet(const std::string& databaseName,
                                                    const std::string& setName,
                                                    const std::string& typeName,
                                                    std::string& errMsg) {
    return simpleRequest<DistributedStorageRemoveTempSet, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not remove temp set to distributed storage manager", errMsg),
        databaseName,
        setName,
        typeName);
}


bool DistributedStorageManagerClient::removeHashSet(const std::string& hashSetName,
                                                    std::string& errMsg) {
    return simpleRequest<DistributedStorageRemoveHashSet, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not remove hash set to distributed storage manager", errMsg),
        hashSetName);
}


bool DistributedStorageManagerClient::exportSet(const std::string& databaseName,
                                                const std::string& setName,
                                                const std::string& outputFilePath,
                                                const std::string& format,
                                                std::string& errMsg) {
    return simpleRequest<DistributedStorageExportSet, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not export set", errMsg),
        databaseName,
        setName,
        outputFilePath,
        format);
}


bool DistributedStorageManagerClient::clearSet(const std::string& databaseName,
                                               const std::string& setName,
                                               const std::string& typeName,
                                               std::string& errMsg) {
    return simpleRequest<DistributedStorageClearSet, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not clear set to distributed storage manager", errMsg),
        databaseName,
        setName,
        typeName);
}


bool DistributedStorageManagerClient::addSharedPage(std::string sharingDatabase,
                   std::string sharingSetName,
                   std::string sharingTypeName,
                   std::string sharedDatabase,
                   std::string sharedSetName,
                   std::string sharedTypeName,
                   PageID pageId,
                   FilePartitionID filePartitionId,
                   unsigned int pageSeqId,
                   bool whetherToAddSharedSet,
                   NodeID nodeId,
		   std::string& errMsg) {

     return simpleRequest<DistributedStorageAddSharedPage, SimpleRequestResult, bool>(
		   logger,
		   port,
		   address,
		   false,
		   1024,
		   generateResponseHandler("Could not add shared page to distributed storage manager", errMsg),   
                   sharingDatabase,
		   sharingSetName,
		   sharingTypeName,
		   sharedDatabase,
		   sharedSetName,
		   sharedTypeName,
		   pageId,
		   filePartitionId,
		   pageSeqId,
                   whetherToAddSharedSet,
		   nodeId);

}

bool DistributedStorageManagerClient::flushData(std::string& errMsg) {
    return simpleRequest<DistributedStorageCleanup, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        generateResponseHandler("Could not cleanup buffered records in distributed storage servers",
                                errMsg));
}


std::function<bool(Handle<SimpleRequestResult>)>
DistributedStorageManagerClient::generateResponseHandler(std::string description,
                                                         std::string& errMsg) {
    return [&](Handle<SimpleRequestResult> result) {
        if (result != nullptr) {
            if (!result->getRes().first) {
                errMsg = description + result->getRes().second;
                logger->error(description + ": " + result->getRes().second);
                return false;
            }
            return true;
        }
        errMsg = "Received nullptr as response";
        logger->error(description + ": received nullptr as response");
        return false;
    };
}
}

#endif
