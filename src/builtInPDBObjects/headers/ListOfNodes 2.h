
#ifndef LIST_OF_NODES_H
#define LIST_OF_NODES_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "PDBVector.h"

using pdb::Vector;

//  PRELOAD %ListOfNodes%

// List of current cluster nodes from distribution manager.
namespace pdb {

class ListOfNodes : public Object {

public:
    ENABLE_DEEP_COPY

    ListOfNodes() {}

    ~ListOfNodes() {}

    Handle<Vector<String>> getHostNames() {
        return hostNames;
    }

    void setHostNames(Handle<Vector<String>> hostNames) {
        this->hostNames = hostNames;
    }

private:
    // hostnames of the nodes
    Handle<Vector<String>> hostNames;
};
}
#endif
