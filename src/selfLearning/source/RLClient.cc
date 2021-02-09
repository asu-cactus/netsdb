#ifndef RL_CLIENT_CC
#define RL_CLIENT_CC

#include "RLClient.h"
#include "PDBServer.h"
#include "json.hpp"
#include <netdb.h>
#include <sys/socket.h> 
#include <netinet/in.h>



namespace pdb {

    RLClient :: RLClient(int port, std::string address, PDBLoggerPtr myLogger) {
        this->port = port;
        this->address = address;
        this->myLogger = myLogger;
        std::cout << "port=" << port << ", address=" << address << std::endl;

    }

    void RLClient::registerHandlers (PDBServer & forMe) {/* no handlers for a RL client */}

    int RLClient::getBestLambdaIndex(RLState& state, double reward, std::string& errMsg) {
        if (state.numCandidates < 0.5) {
            return -1;
        }
        std::vector<double> stateVec;
        state.toVector(stateVec);

        nlohmann::json j;
        j["state"] = stateVec;
        j["reward"] = reward;
        
        std::string message = j.dump();
        std::cout << "JSON message: " << message << std::endl;
        myLogger->warn(message);
        int length = message.length();
        std::cout << "length: " << length << std::endl;
        length = htonl(length);
        //to connect to server
        int sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockFd < 0) {
            std::cout << "Error in opening socket" << std::endl;
            return -1;
        }
        struct hostent *server = gethostbyname(this->address.c_str());
        if (server == nullptr) {
            return -1;
        }
        struct sockaddr_in serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
            (char *)&serv_addr.sin_addr.s_addr,
             server->h_length);
        serv_addr.sin_port = htons(this->port);
        std::cout << "to connect to server:" << this->address <<":" << this->port << std::endl;
        if (connect(sockFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            std::cout << "ERROR connecting" << std::endl;
            return -1;
        }
        //to send length
        std::cout << "to send length:" << length << " with " << sizeof(length) << " bytes" << std::endl;
        //writeMessageSize(sockFd, length);
        write(sockFd, &length, sizeof(length));
        //to send bytes
        std::cout << "to send message" << std::endl;
        doTheWrite(sockFd, (char *)(message.data()), message.length());
        //to receive action
        int actionIndex = -1;
        std::cout << "to receive action index" << std::endl;
        //readActionIndex(sockFd, actionIndex);
        read(sockFd, &actionIndex, sizeof(actionIndex));
        actionIndex = ntohl(actionIndex);
        std::cout << "received " << actionIndex << std::endl;
        myLogger->warn(std::string("received ") + std::to_string(actionIndex));
        //to close connection
        close(sockFd);
        sockFd = -1;

        return actionIndex;

    }

    // to write to network
    bool RLClient::doTheWrite(int sockFd, char* start, size_t length) {
        size_t sizeToWrite = length;
        while (sizeToWrite > 0) {
           ssize_t numBytes = write(sockFd, start, sizeToWrite);
           if(numBytes < 0) {
               std::cout << "Error in socket write: " << strerror(errno) << std::endl;
               std::cout << "Exit with " << sizeToWrite << " bytes to write" << std::endl;
               return true;
           } else {
               sizeToWrite -= numBytes;
               start += numBytes;
               std::cout << "sent " << numBytes << " this time" << std::endl;
           }

        }
        return false;

    }

    bool RLClient::writeMessageSize(int sockFd, size_t length) {

        return doTheWrite(sockFd, (char*)(&length), sizeof(size_t));

    }


    bool RLClient::readActionIndex(int sockFd, int& actionIndex) {
        int receivedBytes = 0;
        if ((receivedBytes = read(sockFd, (char*)(&actionIndex), sizeof(actionIndex))) <= 0) {
            std::cout << "Error in socket read: " << strerror(errno) << std::endl;
            return false;
        }
        return true;    
    }



}


#endif
