
// Added by Leo to test pipeline stuff and to avoid linker issues

#ifndef PIPELINE_DUMMY_TEST_SERVER_H
#define PIPELINE_DUMMY_TEST_SERVER_H

#include "ServerFunctionality.h"
#include "PDBServer.h"
#include <vector>
#include "PDBVector.h"
#include "QueryBase.h"

namespace pdb {

class PipelineDummyTestServer : public ServerFunctionality {

public:
    PipelineDummyTestServer();

    void registerHandlers(PDBServer& forMe) override;

    ~PipelineDummyTestServer();
};
}

#endif
