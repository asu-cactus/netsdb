/*
 * File:   ServerWork.h
 * Author: Chris
 *
 * Created on September 25, 2015, 5:05 PM
 */

#ifndef SERVERWORK_H
#define SERVERWORK_H


#include "PDBBuzzer.h"
#include "PDBCommWork.h"
#include "PDBServer.h"

namespace pdb {

#include <memory>
class ServerWork;
typedef shared_ptr<ServerWork> ServerWorkPtr;

// does all of the work associated with one connection to the server

class ServerWork : public PDBCommWork {
public:
    // specify the server we are working on
    ServerWork(PDBServer& workOnMe);

    // do the actual work
    void execute(PDBBuzzerPtr callerBuzzer) override;

    // error handler
    void handleError();

    // clone this guy
    PDBCommWorkPtr clone() override;

    // get buzzer linked to this guy
    PDBBuzzerPtr getLinkedBuzzer() override;

private:
    // calling handleError sets this to true
    bool wasEnError;

    // the server we are doing the work for
    PDBServer& workOnMe;
};
}

#endif /* SERVERWORK_H */
