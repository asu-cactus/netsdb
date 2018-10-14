
#ifndef FRONTEND_QUERY_TEST_SERVER_H
#define FRONTEND_QUERY_TEST_SERVER_H

#include "ServerFunctionality.h"
#include "QueryBase.h"
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
    void computeQuery(std::string setOutputName,
                      std::string setPrefix,
                      int& whichNode,
                      Handle<QueryBase>& computeMe,
                      std::vector<std::string>& tempSetsCreated);

    // this actually computes a selection query
    void doSelection(std::string setOutputName, Handle<QueryBase>& computeMe);

    int tempSetName;

    bool isStandalone;

    bool createOutputSet;
};
}

#endif
