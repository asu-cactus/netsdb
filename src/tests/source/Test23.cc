
#ifndef TEST_23_CC
#define TEST_23_CC

#include "PDBServer.h"
#include "CatalogServer.h"
#include "CatalogClient.h"
#include "PangeaStorageServer.h"

int main() {

    std::cout << "Starting up a catalog/storage server!!\n";
    pdb::PDBLoggerPtr myLogger = make_shared<pdb::PDBLogger>("frontendLogFile.log");
    pdb::PDBServer frontEnd(8108, 10, myLogger);
    frontEnd.addFunctionality<pdb::CatalogServer>("CatalogDir", true, "localhost", 8108);
    frontEnd.addFunctionality<pdb::CatalogClient>(8108, "localhost", myLogger);

    ConfigurationPtr conf = make_shared<Configuration>();
    pdb::PDBLoggerPtr logger = make_shared<pdb::PDBLogger>(conf->getLogFile());
    SharedMemPtr shm = make_shared<SharedMem>(conf->getShmSize(), logger);
    // pdb :: PDBWorkerQueuePtr workers = make_shared < pdb :: PDBWorkerQueue > (logger,
    // conf->getMaxConnections());
    frontEnd.addFunctionality<pdb::PangeaStorageServer>(
        shm, frontEnd.getWorkerQueue(), logger, conf);
    frontEnd.getFunctionality<pdb::PangeaStorageServer>().startFlushConsumerThreads();
    frontEnd.startServer(nullptr);
}

#endif
