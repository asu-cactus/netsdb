#ifndef DISTRIBUTION_MANAGER_SERVER_H
#define DISTRIBUTION_MANAGER_SERVER_H

#include "ServerFunctionality.h"
#include "PDBServer.h"
#include "PDBDistributionManager.h"
#include "Handle.h"
#include "QueryBase.h"
#include "PDBVector.h"
#include "PDBLogger.h"

namespace pdb {

class DistributionManagerServer : public ServerFunctionality {

public:
    // these give us the port and the address of the catalog
    DistributionManagerServer(PDBDistributionManagerPtr distributionManagerIn);

    ~DistributionManagerServer();

    // from the ServerFunctionality interface
    void registerHandlers(PDBServer& forMe) override;

    // This method adds or update a node.
    // If the node is seen for the first time it adds it to memory with the current time and returns
    // 0
    // If the node already exists it updates the timestamp and returns 1.
    int addOrUpdateNodes(PDBLoggerPtr myLoggerIn, string& nodeID);

    PDBDistributionManagerPtr getDistributionManager();

    void setDistributionManager(PDBDistributionManagerPtr distributionManagerIn);
    PDBLoggerPtr getLogger();

private:
    PDBDistributionManagerPtr distributionManager;
    PDBLoggerPtr logToMe;
};
}

#endif
