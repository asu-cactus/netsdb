#ifndef OBJECTQUERYMODEL_DISPATCHERCLIENT_CC
#define OBJECTQUERYMODEL_DISPATCHERCLIENT_CC

#include "DispatcherClient.h"
#include "SimpleRequest.h"
#include "DispatcherRegisterPartitionPolicy.h"

namespace pdb {

DispatcherClient::DispatcherClient() {};

DispatcherClient::DispatcherClient(int portIn, std::string addressIn, PDBLoggerPtr myLoggerIn)
    : myHelper(portIn, addressIn, myLoggerIn) {
    this->logger = myLoggerIn;
    this->port = portIn;
    this->address = addressIn;
}

DispatcherClient::~DispatcherClient() {}

void DispatcherClient::registerHandlers(PDBServer& forMe) {}  // no-op

bool DispatcherClient::registerSet(std::pair<std::string, std::string> setAndDatabase,
                                   PartitionPolicy::Policy policy,
                                   std::string& errMsg) {

    return simpleRequest<DispatcherRegisterPartitionPolicy, SimpleRequestResult, bool>(
        logger,
        port,
        address,
        false,
        1024,
        [&](Handle<SimpleRequestResult> result) {
            if (result != nullptr) {
                if (!result->getRes().first) {
                    errMsg = "Error registering partition policy for " + setAndDatabase.first +
                        ":" + setAndDatabase.second + ": " + result->getRes().second;
                    logger->error(errMsg);
                    return false;
                }
                return true;
            }
            errMsg =
                "Error registering partition policy: got nothing back from the DispatcherServer";
            return false;
        },
        setAndDatabase.first,
        setAndDatabase.second,
        policy);
}

pair<string, string> DispatcherClient::MM_getSet(const std::string &dbName, const std::string &setName, std::string &errMsg) {

    // make a request and return the value
    return simpleRequest<DispatcherGetSetRequest, DispatcherGetSetResult, pair<string, string>>(
              logger, port, address, pair<string, string>("",""), 1024,
              [&](Handle<DispatcherGetSetResult> result) {

                // do we have the thing
                if(result != nullptr && result->databaseName == dbName && result->setName == setName) {
                  return pdb::makeObject<pair<string, string>>(result->databaseName, result->setName);
                }

                // otherwise
                return pair<string, string>("","");
              },
              dbName, setName);
}

}

#include "StorageClientTemplate.cc"

#endif
