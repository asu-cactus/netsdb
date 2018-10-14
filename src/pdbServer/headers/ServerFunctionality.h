
#ifndef SERVER_FUNCT_H
#define SERVER_FUNCT_H

#include "PDBServer.h"

namespace pdb {

// this pure virtual class encapsulates some particular server functionality (catalog client,
// catalog server, storage server, etc.).
class ServerFunctionality {

public:
    // registers any particular handlers that this server needs
    virtual void registerHandlers(PDBServer& forMe) = 0;

    // added by Jia, it will be invoked when PDBServer is to be shutdown
    virtual void cleanup() {}

    // access a particular functionality on the attached server
    template <class Functionality>
    Functionality& getFunctionality() {
        return parent->getFunctionality<Functionality>();
    }

    // remember the server this is attached to
    void recordServer(PDBServer& recordMe) {
        parent = &recordMe;
    }

    PDBWorkerPtr getWorker() {
        return parent->getWorkerQueue()->getWorker();
    }

    PDBLoggerPtr getLogger() {
        return parent->getLogger();
    }


private:
    PDBServer* parent;
};
}

#endif
