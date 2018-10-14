#ifndef CAT_REMOVE_NODE_FROM_DB_H
#define CAT_REMOVE_NODE_FROM_DB_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatRemoveNodeFromDatabaseRequest%

namespace pdb {

// encapsulates a request to remove a node from a database
class CatRemoveNodeFromDatabaseRequest : public Object {

public:
    ~CatRemoveNodeFromDatabaseRequest() {}
    CatRemoveNodeFromDatabaseRequest() {}
    CatRemoveNodeFromDatabaseRequest(std::string dbName, std::string nodeIP)
        : dbName(dbName), nodeIP(nodeIP) {}

    std::string nodeToRemove() {
        return nodeIP;
    }

    std::string whichDB() {
        return dbName;
    }


    ENABLE_DEEP_COPY

private:
    String dbName;
    String nodeIP;
};
}

#endif
