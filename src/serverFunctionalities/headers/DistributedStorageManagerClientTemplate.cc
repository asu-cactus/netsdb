
#ifndef DISTRIBUTED_STORAGE_MANAGER_CLIENT_TEMPLATE_CC
#define DISTRIBUTED_STORAGE_MANAGER_CLIENT_TEMPLATE_CC

#include "PDBDebug.h"
#include "DistributedStorageManagerClient.h"
#include "StorageAddSet.h"
#include "SimpleRequest.h"
#include "DistributedStorageAddSet.h"
#include "DistributedStorageAddSetWithPartition.h"
#include "SimpleRequestResult.h"
#include "DataTypes.h"
#include <cstddef>
#include <fcntl.h>
#include <fstream>
#include <iostream>

namespace pdb {


    template <class DataType>
    bool DistributedStorageManagerClient::createSet(const std::string& databaseName,
                                                    const std::string& setName,
                                                    std::string& errMsg,
                                                    size_t pageSize,
                                                    const std::string& createdJobId, 
                                                    Handle<Computation> computationForDispatch,
                                                    Handle<LambdaIdentifier> lambdaForDispatch,
                                                    size_t desiredSize,
                                                    bool isMRU, bool share) {
        std::string typeName = getTypeName<DataType>();
        int16_t typeId = getTypeID<DataType>();
        PDB_COUT << "typeName for set to create =" << typeName << ", typeId=" << typeId << std::endl;
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
            computationForDispatch,
            lambdaForDispatch,
            desiredSize,
            isMRU,
            share);
    }

    template <class DataType>
    bool DistributedStorageManagerClient::createSet(const std::string& databaseName,
                                                    const std::string& setName,
                                                    std::string& errMsg,
                                                    size_t pageSize,
                                                    const std::string& createdJobId,
                                                    Handle<Vector<Handle<Computation>>> computationsForDispatch,
                                                    std::string jobName,
                                                    std::string jobName1, 
                                                    std::string jobName2, 
                                                    std::string computationName1, 
                                                    std::string computationName2, 
                                                    std::string lambdaName1, 
                                                    std::string lambdaName2,
                                                    size_t desiredSize,
                                                    bool isMRU, bool share) {
        std::string typeName = getTypeName<DataType>();
        int16_t typeId = getTypeID<DataType>();
        PDB_COUT << "typeName for set to create =" << typeName << ", typeId=" << typeId << std::endl;
        std::cout << "to create set for " << databaseName << ":" << setName << std::endl;
        std::cout << "jobName is " << jobName << std::endl;
        Handle<DistributedStorageAddSetWithPartition> request = makeObject<DistributedStorageAddSetWithPartition> (databaseName,
            setName, typeName, pageSize, createdJobId, computationsForDispatch, jobName, jobName1, jobName2, computationName1, computationName2, lambdaName1, lambdaName2, desiredSize, isMRU, share);
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
           computationsForDispatch);
       }

}
#endif
