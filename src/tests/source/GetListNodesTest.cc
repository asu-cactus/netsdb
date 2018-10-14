#ifndef GETLISTOFNODESTEST_CC
#define GETLISTOFNODESTEST_CC

#include <iostream>
#include <vector>
#include "PDBServer.h"
#include "DistributionManagerServer.h"
#include "DistributionManagerClient.h"

typedef vector<string> CommandLineStringArgs;

using namespace pdb;
using namespace std;

// Cluster Heart Beat Test.
int main(int argc, char* argv[]) {

    std::cout << "start the cluster then \n";
    std::cout << "start:  ./bin/getListOfNodesTest port MasterNodeHostName frontEndDM.log    \n";

    CommandLineStringArgs cmdlineStringArgs(&argv[0], &argv[0 + argc]);

    int portNumber;
    std::string hostname;
    std::string logfile;

    bool wasError;
    std::string errMsg;

    if (argc == 4) {
        portNumber = atoi(argv[1]);
        hostname = std::string(cmdlineStringArgs[2]);
        logfile = cmdlineStringArgs[3];
    }
    else {
        cout << "Usage : portNumber, masterNodeHostName, logFile" << "\n";
        cout << "Using default values : 8108, master, master.log" << "\n";

        portNumber = 8108;
        hostname = "localhost";
        logfile = "master.log";
    }

    pdb::PDBLoggerPtr myLogger = make_shared<pdb::PDBLogger>(logfile);
    DistributionManagerClientPtr myDMClient = make_shared<DistributionManagerClient>(myLogger);

    Handle<ListOfNodes> listOfNodes = myDMClient->getCurrentNodes(hostname, portNumber, wasError, errMsg);
    Handle<Vector<String>> myVectorOfNodes = listOfNodes->getHostNames();

    cout << "List of Nodes are:\n";
    // Now Iterate over the vector and get the current list of nodes
    for (int var = 0; var < myVectorOfNodes->size(); ++var) {
        cout << string(myVectorOfNodes->c_ptr()[var].c_str()) << endl;
    }

    std::cout << errMsg << std::endl;
}

#endif
