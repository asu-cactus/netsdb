
#ifndef PANGEA_QUERY_SERVER_H
#define PANGEA_QUERY_SERVER_H

#include "ServerFunctionality.h"
#include "PDBServer.h"
#include <vector>
#include "PDBVector.h"
#include "QueryBase.h"

namespace pdb {

class PangeaQueryServer : public ServerFunctionality {

public:
    // creates a query server... the param is the number of threads to use
    // to answer queries
    PangeaQueryServer(int numThreads);

    // from the ServerFunctionality interface... registers the PangeaQueryServer's
    // handlers
    void registerHandlers(PDBServer& forMe) override;

    // this recursively traverses a simple query graph, where each node can only have one input,
    // makes sure that each node has been computed... if setOutputName is equal to "", then
    // the parameter setPrefix is the string that we'll use
    // to create each set name, whichNode is the counter that we use to name each set, and
    // computeMe is the node that we are wirred about computing
    void computeQuery(std::string setOutputName,
                      std::string setPrefix,
                      int& whichNode,
                      Handle<QueryBase>& computeMe,
                      std::vector<std::string>& tempSetsCreated);

    // this actually computes a selection query
    void doSelection(std::string setOutputName, Handle<QueryBase>& computeMe);

    // destructor
    ~PangeaQueryServer();

private:
    // the number of threads to use
    int numThreadsToUse;

    // used to count up temporary file names: tempSet0, tempSet1, tempSet2, ...
    int tempSetName;
};
}

#endif
