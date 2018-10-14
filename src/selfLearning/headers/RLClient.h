
#ifndef RL_CLIENT_H
#define RL_CLIENT_H


#include "ServerFunctionality.h"
#include "CatalogClient.h"
#include "PDBLogger.h"
#include "PDBServer.h"
#include "LambdaIdentifier.h"
#include "RLState.h"

namespace pdb {


//this class encapsulates a RLClient to talk with RLServer (implemented in python)

class RLClient : public ServerFunctionality {

public:
    // these give us the port and the address of the RLServer
    RLClient(int port, std::string address, PDBLoggerPtr myLogger);

    // function to register event handlers associated with this server functionality
    virtual void registerHandlers(PDBServer& forMe) override;

    // transform state and last reward into JSON format, send it to RL server, and obtain optimal partition lambda
    int getBestLambdaIndex(RLState& state, double reward, std::string& errMsg);

    // to write to network
    bool doTheWrite(int sockFd, char* start, size_t length);

    // to write message size to the network
    bool writeMessageSize(int sockFd, size_t length);

    // to read action index from network
    bool readActionIndex(int sockFd, int& actionIndex);


private:

    int port;

    std::string address;

    PDBLoggerPtr myLogger;

};


}

#endif
