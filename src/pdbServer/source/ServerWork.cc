
#ifndef SERVER_WORK_CC
#define SERVER_WORK_CC

#include "ServerWork.h"

namespace pdb {

ServerWork::ServerWork(PDBServer& workOnMeIn) : workOnMe(workOnMeIn) {
    wasEnError = false;
}

PDBCommWorkPtr ServerWork::clone() {
    PDBCommWorkPtr returnVal{make_shared<ServerWork>(workOnMe)};
    return returnVal;
}

void ServerWork::handleError() {
    wasEnError = true;
    getLogger()->error("ServerWork: got an error");
}

PDBBuzzerPtr ServerWork::getLinkedBuzzer() {
    return std::make_shared<PDBBuzzer>([&](PDBAlarm myAlarm) {});
}

void ServerWork::execute(PDBBuzzerPtr callerBuzzer) {

    // while there is still something to do on this connection
    getLogger()->trace("ServerWork: about to handle a request");
    PDBBuzzerPtr myBuzzer{getLinkedBuzzer()};
    while (!wasEnError && workOnMe.handleOneRequest(myBuzzer, getCommunicator())) {
        getLogger()->trace("ServerWork: just handled another request");
        myBuzzer = getLinkedBuzzer();
    }

    getLogger()->trace("ServerWork: done with this server work");
    callerBuzzer->buzz(PDBAlarm::WorkAllDone);
}
}

#endif
