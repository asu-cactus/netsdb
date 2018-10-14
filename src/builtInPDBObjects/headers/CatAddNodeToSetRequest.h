
#ifndef CAT_ADD_NODE_TO_SET_H
#define CAT_ADD_NODE_TO_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatAddNodeToSetRequest%

namespace pdb {

// encapsulates a request to add a node to an existing set
class CatAddNodeToSetRequest : public Object {

public:
    ~CatAddNodeToSetRequest() {}
    CatAddNodeToSetRequest() {}
    CatAddNodeToSetRequest(std::string dbName, std::string setName, std::string nodeIP)
        : dbName(dbName), setName(setName), nodeIP(nodeIP) {}

    std::string nodeToAdd() {
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
