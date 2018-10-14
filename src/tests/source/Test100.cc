#ifndef TEST_100_CC
#define TEST_100_CC

#include <iostream>
#include <vector>
#include <string>

#include "PDBServer.h"
#include "DistributionManagerServer.h"
#include "DistributionManagerClient.h"

#include "InterfaceFunctions.h"
#include "NodeInfo.h"

typedef vector<string> CommandLineStringArgs;

using namespace pdb;

// Cluster Heart Beat Test.

int main(int argc, char* argv[]) {

    std::cout << "On two command Line. Run commands like\n";
    std::cout << "start:  ./bin/test100 8108 frontEndDM.log localhost 8108 1 \n";
    std::cout << "start:  ./bin/test100 8109 frontEndDM1.log localhost 8108 0 \n";

    CommandLineStringArgs cmdlineStringArgs(&argv[0], &argv[0 + argc]);

    if (argc == 6) {

        // TODO : if we need to work with the full hostname.
        //		char hostname[1024];
        //		hostname[1023] = '\0';
        //		gethostname(hostname, 1023);
        ////	printf("Hostname: %s\n", hostname);
        //
        //// /* Description of data base entry for a single host.  */
        //		struct hostent* h;
        //		h = gethostbyname(hostname);
        //
        //		// we need the full hostname.
        //		std::string myhostname = h->h_name;
        //		printf("full hostname is: %s\n", h->h_name);

        int portNumber = atoi(argv[1]);

        std::string logfile = cmdlineStringArgs[2];

        std::string masterNodeHostname = cmdlineStringArgs[3];
        int masterPortNumber = atoi(argv[4]);
        int isMaster = atoi(argv[5]);

        std::cout << "Server Port Number is: " << portNumber << ". Master flag is: " << isMaster
                  << std::endl;

        pdb::PDBLoggerPtr myLogger = make_shared<pdb::PDBLogger>(logfile);
        pdb::PDBServer frontEnd(portNumber, 10, myLogger);

        PDBDistributionManagerPtr myDM = make_shared<PDBDistributionManager>();
        frontEnd.addFunctionality<pdb::DistributionManagerServer>(myDM);

        cout << "Master node hostname is " << masterNodeHostname << "  port is " << masterPortNumber
             << endl;

        if (isMaster == 0) {
            std::cout << "Server is a Slave Node." << std::endl;


            bool wasError;
            std::string errMsg;


            pdb::String hostname("localhost");

            frontEnd.addFunctionality<pdb::DistributionManagerClient>(
                hostname, portNumber, frontEnd.getLogger());

            // Get the functionality back to start the heart beat.
            pdb::DistributionManagerClient myDMClient =
                frontEnd.getFunctionality<pdb::DistributionManagerClient>();

            myDMClient.sendHeartBeat(masterNodeHostname, masterPortNumber, wasError, errMsg);
            std::cout << errMsg << std::endl;
        }

        frontEnd.startServer(nullptr);
    } else {
        cout << "Provide: portNumber, masterNodeHostName, masterNodePort,  and a integer:  1 for "
                "MasterNode, 0 for SlaveNode"
             << "\n";
    }
}

#endif
