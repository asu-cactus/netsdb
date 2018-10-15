
#ifndef DISTRIBUTED_STORAGE_MANAGER_CLIENT_TEMPLATE_CC
#define DISTRIBUTED_STORAGE_MANAGER_CLIENT_TEMPLATE_CC

#include "PDBDebug.h"
#include "DistributedStorageManagerClient.h"
#include "StorageAddSet.h"
#include "SimpleRequest.h"
#include "DistributedStorageAddSet.h"
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
                                                    size_t desiredSize) {
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
            desiredSize);
    }

}
#endif
