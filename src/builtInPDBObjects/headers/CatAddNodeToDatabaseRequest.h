
#ifndef CAT_ADD_NODE_TO_DB_H
#define CAT_ADD_NODE_TO_DB_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatAddNodeToDatabaseRequest%

namespace pdb {

// encapsulates a request to add a node to an existing database
class CatAddNodeToDatabaseRequest : public Object {

public:
    ~CatAddNodeToDatabaseRequest() {}
    CatAddNodeToDatabaseRequest() {}
    CatAddNodeToDatabaseRequest(std::string dbName, std::string nodeIP)
        : dbName(dbName), nodeIP(nodeIP) {}

    std::string nodeToAdd() {
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
