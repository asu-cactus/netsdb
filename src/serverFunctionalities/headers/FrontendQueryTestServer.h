
#ifndef FRONTEND_QUERY_TEST_SERVER_H
#define FRONTEND_QUERY_TEST_SERVER_H

#include "ServerFunctionality.h"
#include "PDBServer.h"

namespace pdb {

/*
 * This server works in frontend as a message proxy to transfer message 
 * from QuerySchedulerServer to HermesExecutionServer.
 */

class FrontendQueryTestServer : public ServerFunctionality {

public:
    FrontendQueryTestServer();

    FrontendQueryTestServer(bool isStandalone, bool createOutputSet);

    void registerHandlers(PDBServer& forMe) override;

    // destructor
    ~FrontendQueryTestServer();

private:

    int tempSetName;

    bool isStandalone;

    bool createOutputSet;
};
}

#endif
