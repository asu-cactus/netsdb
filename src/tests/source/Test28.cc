
#ifndef TEST_28_CC
#define TEST_28_CC

#include "PDBServer.h"
#include "CatalogServer.h"
#include "CatalogClient.h"
#include "StorageClient.h"
#include "PangeaStorageServer.h"
#include "HermesExecutionServer.h"

int main() {

    std::cout << "Starting up a frontend server!!\n";
    pdb::PDBLoggerPtr logger = make_shared<pdb::PDBLogger>("frontendLogFile.log");
    ConfigurationPtr conf = make_shared<Configuration>();
    SharedMemPtr shm = make_shared<SharedMem>(conf->getShmSize(), logger);

    if (shm != nullptr) {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            // I'm the backend server
            pdb::PDBLoggerPtr logger = make_shared<pdb::PDBLogger>("backendLogFile.log");
            pdb::PDBServer backEnd(conf->getBackEndIpcFile(), 100, logger);
            backEnd.addFunctionality<pdb::HermesExecutionServer>(
                shm, backEnd.getWorkerQueue(), logger, conf);
            bool usePangea = true;
            backEnd.addFunctionality<pdb::StorageClient>(
                8108, "localhost", make_shared<pdb::PDBLogger>("clientLog"), usePangea);
            backEnd.startServer(nullptr);

        } else if (child_pid == -1) {
            std::cout << "Fatal Error: fork failed." << std::endl;
        } else {
            // I'm the frontend server
            pdb::PDBServer frontEnd(8108, 100, logger);
            frontEnd.addFunctionality<pdb::CatalogServer>("CatalogDir", false, "localhost", 8108);
            frontEnd.addFunctionality<pdb::CatalogClient>(8108, "localhost", logger);
            frontEnd.addFunctionality<pdb::PangeaStorageServer>(
                shm, frontEnd.getWorkerQueue(), logger, conf);
            frontEnd.getFunctionality<pdb::PangeaStorageServer>().startFlushConsumerThreads();
            frontEnd.startServer(nullptr);
        }
    }
}

#endif
