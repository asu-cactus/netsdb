
#ifndef PDB_SERVER_TEMP_CC
#define PDB_SERVER_TEMP_CC

#include "Handle.h"
#include "PDBServer.h"
#include "ServerFunctionality.h"
#include <memory>

namespace pdb {

template <class Functionality, class... Args>
void PDBServer::addFunctionality(Args&&... args) {

    // first, get the name of this type
    std::string myType = getTypeName<Functionality>();

    // and remember him... map him to a particular index in the list of functionalities
    if (allFunctionalityNames.count(myType) == 1) {
        std::cerr << "BAD!  You can't add the same functionality twice.\n";
    }
    allFunctionalityNames[myType] = allFunctionalities.size();

    // then create the functionality
    shared_ptr<ServerFunctionality> whichFunctionality = make_shared<Functionality>(args...);
    allFunctionalities.push_back(whichFunctionality);

    registerHandlersFromLastFunctionality();
}

template <class Functionality>
Functionality& PDBServer::getFunctionality() {

    // first, figure out which index we are
    static int whichIndex = -1;
    if (whichIndex == -1) {
        std::string myType = getTypeName<Functionality>();
        whichIndex = allFunctionalityNames[myType];
    }

    // and now, return the functionality
    return *((Functionality*)allFunctionalities[whichIndex].get());
}
}

#endif
