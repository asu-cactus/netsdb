#ifndef MASTER_MAIN_CC
#define MASTER_MAIN_CC

#include <iostream>
#include <string>

#include "PDBServer.h"
#include "CatalogServer.h"
#include "CatalogClient.h"
#include "ResourceManagerServer.h"
#include "PangeaStorageServer.h"
#include "DistributedStorageManagerServer.h"
#include "DispatcherServer.h"
#include "QuerySchedulerServer.h"
#include "StorageAddDatabase.h"
#include "SharedEmployee.h"
#include "SelfLearningServer.h"
#include "Configuration.h"

int main(int argc, char* argv[]) {
    std::cout << "running main function..." << std::endl;
    int port = 8108;
    std::string masterIp;
    std::string pemFile = "conf/pdb.key";
    bool pseudoClusterMode = false;
    double partitionToCoreRatio = 1;
    bool trainingMode = false;
    if (argc == 3) {
        masterIp = argv[1];
        port = atoi(argv[2]);
    } else if ((argc == 4) || (argc == 5) || (argc == 6) || (argc == 7)) {
        masterIp = argv[1];
        port = atoi(argv[2]);
        std::string isPseudoStr(argv[3]);
        if (isPseudoStr.compare(std::string("Y")) == 0) {
            pseudoClusterMode = true;
            std::cout << "Running in pseudo cluster mode" << std::endl;
        }
        if ((argc == 5) || (argc == 6) || (argc ==7)) {
            pemFile = argv[4];
        }
        if ((argc == 6) || (argc == 7)){
            partitionToCoreRatio = stod(argv[5]);
        }
        if (argc == 7) {
            std::string isTrainingStr(argv[6]);
            if (isTrainingStr.compare(std::string("Y")) == 0) {
                trainingMode = true;
                std::cout << "Running in training mode" << std::endl;
            }
        }

    } else {
        std::cout << "[Usage] #masterIp #port #runPseudoClusterOnOneNode (Y for running a "
                     "pseudo-cluster on one node, N for running a real-cluster distributedly, and "
                     "default is N) #pemFile (by default is conf/pdb.key) #partitionToCoreRatio "
                     "(by default is 1)"
                  << std::endl;
        exit(-1);
    }
    
    bool isSelfLearning = false;

    std::cout << "Starting up a distributed storage manager server\n";
    pdb::PDBLoggerPtr myLogger = make_shared<pdb::PDBLogger>("frontendLogFile.log");
    std::cout << "Starting server...\n" << std::endl; 
    pdb::PDBServer frontEnd(port, 100, myLogger);

    ConfigurationPtr conf = make_shared<Configuration>();

    std::cout << "Creating catalog server" << std::endl;
    frontEnd.addFunctionality<pdb::CatalogServer>("CatalogDir", true, masterIp, port, masterIp, port);

    std::cout << "Creating catalog client" << std::endl;
    frontEnd.addFunctionality<pdb::CatalogClient>(port, "localhost", myLogger);

    // to register node metadata
    std::string errMsg = " ";
    string serverListFile = "";

    if (pseudoClusterMode==true) serverListFile = "conf/serverlist.test";
    else serverListFile = "conf/serverlist";


    int numNodes = 1;
    string line = "";
    string nodeName = "";
    string hostName = "";
    int portValue = 8108;

    pdb::PDBLoggerPtr rlLogger = std::make_shared<pdb::PDBLogger>("rlClient.log");
    std::cout << "starting resource manager server" << std::endl;
    frontEnd.addFunctionality<pdb::ResourceManagerServer>(
        serverListFile, port, pseudoClusterMode, pemFile);
    std::cout << "starting distributed storage manager server" << std::endl;
    frontEnd.addFunctionality<pdb::DistributedStorageManagerServer>(rlLogger, isSelfLearning, trainingMode);
    auto allNodes = frontEnd.getFunctionality<pdb::ResourceManagerServer>().getAllNodes();
    std::cout << "starting dispatcher server" << std::endl;
    frontEnd.addFunctionality<pdb::DispatcherServer>(myLogger, isSelfLearning);
    frontEnd.getFunctionality<pdb::DispatcherServer>().registerStorageNodes(allNodes);
    std::cout << "starting query scheduling server" << std::endl;
    frontEnd.addFunctionality<pdb::QuerySchedulerServer>(
        port, myLogger, conf, pseudoClusterMode, partitionToCoreRatio, true, false, isSelfLearning);
    std::cout << "starting self learning server" << std::endl;
    frontEnd.addFunctionality<pdb::SelfLearningServer>(conf, port);
    std::cout << "starting server" << std::endl;
    frontEnd.startServer(nullptr);
}

#endif
