#ifndef MASTER_SERVER_TEST_CC
#define MASTER_SERVER_TEST_CC

#include "PDBServer.h"
#include "CatalogServer.h"
#include "CatalogClient.h"
#include "ResourceManagerServer.h"
#include "DistributedStorageManagerServer.h"
#include "DispatcherServer.h"

/**
 * Starts up a server with all of the master functionalities and performs all the proper
 * initializations for the various
 * servers
 *
 * Usage: ./MasterServer <port>
 */
int main(int argc, char* argv[]) {

    int port = 8108;
    if (argc >= 2) {
        port = atoi(argv[1]);
    }

    std::cout << "Starting up a master server" << std::endl;
    pdb::PDBLoggerPtr myLogger = make_shared<pdb::PDBLogger>("frontendLogFile.log");
    pdb::PDBServer frontEnd(port, 10, myLogger);
    frontEnd.addFunctionality<pdb::CatalogServer>("CatalogDir", true, "localhost", 8108);
    frontEnd.addFunctionality<pdb::CatalogClient>(port, "localhost", myLogger);
    frontEnd.addFunctionality<pdb::ResourceManagerServer>("conf/serverlist", port);
    frontEnd.addFunctionality<pdb::DistributedStorageManagerServer>(myLogger);
    frontEnd.addFunctionality<pdb::DispatcherServer>(myLogger);
    // Register the nodes into the dispatcher
    auto allNodes = frontEnd.getFunctionality<pdb::ResourceManagerServer>().getAllNodes();
    frontEnd.getFunctionality<pdb::DispatcherServer>().registerStorageNodes(allNodes);
    frontEnd.startServer(nullptr);
}

#endif
