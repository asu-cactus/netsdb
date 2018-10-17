#ifndef OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERCLIENT_CC
#define OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEMANAGERCLIENT_CC

#include "DistributedStorageManagerClient.h"

#include "SimpleRequest.h"
#include "DistributedStorageAddDatabase.h"
#include "DistributedStorageAddSet.h"
#include "DistributedStorageAddTempSet.h"
#include "DistributedStorageRemoveDatabase.h"
#include "DistributedStorageRemoveSet.h"
#include "DistributedStorageRemoveTempSet.h"
#include "DistributedStorageExportSet.h"
#include "DistributedStorageClearSet.h"
#include "DistributedStorageCleanup.h"

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
                                                size_t desiredSize ) {
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
        desiredSize);
}

bool DistributedStorageManagerClient::createTempSet(const std::string& databaseName,
                                                    const std::string& setName,
                                                    const std::string& typeName,
                                                    std::string& errMsg,
                                                    size_t pageSize,
                                                    const std::string& createdJobId,
                                                    Handle<Computation> dispatchComputation,
                                                    Handle<LambdaIdentifier> lambdaIdentifier,
                                                    size_t desiredSize ) {
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
        desiredSize);
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
