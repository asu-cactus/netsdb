#ifndef CAT_REMOVE_NODE_FROM_SET_H
#define CAT_REMOVE_NODE_FROM_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatRemoveNodeFromSetRequest%

namespace pdb {

// encapsulates a request to remove a node from a Set
class CatRemoveNodeFromSetRequest : public Object {

public:
    ~CatRemoveNodeFromSetRequest() {}
    CatRemoveNodeFromSetRequest() {}
    CatRemoveNodeFromSetRequest(std::string dbName, std::string setName, std::string nodeIP)
        : dbName(dbName), setName(setName), nodeIP(nodeIP) {}

    std::string nodeToRemove() {
        return nodeIP;
    }

    std::string whichSet() {
        return setName;
    }

    std::string whichDB() {
        return dbName;
    }

    ENABLE_DEEP_COPY

private:
    String dbName;
    String setName;
    String nodeIP;
};
}

#endif
