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
}

#include "StorageClientTemplate.cc"

#endif
