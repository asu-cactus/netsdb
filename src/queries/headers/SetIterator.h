
#ifndef SET_ITER_H
#define SET_ITER_H

#include "OutputIterator.h"
#include "SetScan.h"
#include <snappy.h>

namespace pdb {

template <class OutType>
class SetIterator {

public:
    // constructor; this should only be used by the query client
    SetIterator(PDBLoggerPtr loggerIn,
                int portIn,
                std::string& serverNameIn,
                std::string& dbNameIn,
                std::string& setNameIn,
		bool isSharedIn = false) {
        myLogger = loggerIn;
        port = portIn;
        serverName = serverNameIn;
        dbName = dbNameIn;
        setName = setNameIn;
	isShared = isSharedIn;
        wasError = false;
	
    }

    SetIterator() {
        wasError = true;
    }

    ~SetIterator() {}

    // this basically sets up a connection to the server, and returns it
    OutputIterator<OutType> begin() {

        // if there was an error, just get outta here
        if (wasError) {
            std::cout << "You are trying to create an iterator when there was an error.\n";
            return OutputIterator<OutType>();
        }

        // establish a connection
        std::string errMsg;
        PDBCommunicatorPtr temp = std::make_shared<PDBCommunicator>();
        if (temp->connectToInternetServer(myLogger, port, serverName, errMsg)) {
            myLogger->error(errMsg);
            myLogger->error("output iterator: not able to connect to server.\n");
            return OutputIterator<OutType>();
        }

        // build the request
        const UseTemporaryAllocationBlock tempBlock{1024};
        Handle<SetScan> request = makeObject<SetScan>(dbName, setName, isShared);
        if (!temp->sendObject(request, errMsg)) {
            myLogger->error(errMsg);
            myLogger->error("output iterator: not able to send request to server.\n");
            return OutputIterator<OutType>();
        }
        PDB_COUT << "sent SetScan object to master" << std::endl;
        return OutputIterator<OutType>(temp);
    }

    OutputIterator<OutType> end() {
        return OutputIterator<OutType>();
    }

private:
    // these are used so that the output knows how to connect to the server for iteration
    int port;
    std::string serverName;
    PDBLoggerPtr myLogger;

    // records the place where the input comes from
    std::string dbName;
    std::string setName;

    // true if there is an error
    bool wasError;

    // true if the set to be scanned links to the shared pages
    bool isShared;


    // allows creation of these objects
    friend class QueryClient;



};
}

#endif
