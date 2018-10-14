
#ifndef STORAGE_CLIENT_CC
#define STORAGE_CLIENT_CC

#include "StorageClient.h"
#include "StorageAddDatabase.h"
#include "StorageCleanup.h"
namespace pdb {

StorageClient::StorageClient(int portIn,
                             std::string addressIn,
                             PDBLoggerPtr myLoggerIn,
                             bool usePangeaIn)
    : myHelper(portIn, addressIn, myLoggerIn) {

    // get the communication information
    port = portIn;
    address = addressIn;
    myLogger = myLoggerIn;
    usePangea = usePangeaIn;
}

void StorageClient::registerHandlers(PDBServer& forMe) { /* no handlers for a storage client!! */
}

bool StorageClient::registerType(std::string regMe, std::string& errMsg) {
    return myHelper.registerType(regMe, errMsg);
}

bool StorageClient::shutDownServer(std::string& errMsg) {

    return myHelper.shutDownServer(errMsg);
}

bool StorageClient::flushData(std::string& errMsg) {
    return simpleRequest<StorageCleanup, SimpleRequestResult, bool>(
        myLogger, port, address, false, 1024, [&](Handle<SimpleRequestResult> result) {
            if (result != nullptr) {
                if (!result->getRes().first) {
                    errMsg = "Error cleanup buffered records in storage server: " +
                        result->getRes().second;
                    myLogger->error("Error cleanup buffered records in storage server: " +
                                    result->getRes().second);
                    return false;
                }
                return true;
            }
            errMsg = "Error cleanup buffered records in storage server";
            return false;
        });
}


bool StorageClient::createDatabase(std::string databaseName, std::string& errMsg) {
    if (usePangea == false) {
        return myHelper.createDatabase(databaseName, errMsg);
    } else {
        return simpleRequest<StorageAddDatabase, SimpleRequestResult, bool>(
            myLogger,
            port,
            address,
            false,
            1024,
            [&](Handle<SimpleRequestResult> result) {
                if (result != nullptr) {
                    if (!result->getRes().first) {
                        errMsg = "Error creating database: " + result->getRes().second;
                        myLogger->error("Error creating database: " + result->getRes().second);
                        return false;
                    }
                    return true;
                }
                errMsg = "Error getting type name, nothing is back from storage";
                return false;
            },
            databaseName);
    }
}

bool StorageClient::storeData(Handle<Vector<Handle<Object>>> data,
                              std::string databaseName,
                              std::string setName,
                              std::string typeName,
                              std::string& errMsg) {
    return simpleSendDataRequest<StorageAddData, Handle<Object>, SimpleRequestResult, bool>(
        myLogger,
        port,
        address,
        false,
        1024,
        [&](Handle<SimpleRequestResult> result) {
            if (result != nullptr)
                if (!result->getRes().first) {
                    myLogger->error("Error sending data: " + result->getRes().second);
                    errMsg = "Error sending data: " + result->getRes().second;
                }
            return true;
        },
        data,
        databaseName,
        setName,
        typeName,
        true);
}


std::string StorageClient::getObjectType(std::string databaseName,
                                         std::string setName,
                                         std::string& errMsg) {
    return myHelper.getObjectType(databaseName, setName, errMsg);
}
}
#endif
