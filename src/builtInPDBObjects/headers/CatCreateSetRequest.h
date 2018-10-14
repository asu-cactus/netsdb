
#ifndef CAT_CREATE_SET_H
#define CAT_CREATE_SET_H

#include "Object.h"
#include "PDBString.h"
#include "Handle.h"

// PRELOAD %CatCreateSetRequest%

namespace pdb {

// encapsulates a request to create a set
class CatCreateSetRequest : public Object {

public:
    ~CatCreateSetRequest() {}
    CatCreateSetRequest() {}
    CatCreateSetRequest(std::string dbName, std::string setName, int16_t typeID)
        : dbName(dbName), setName(setName), typeID(typeID) {}

    std::pair<std::string, std::string> whichSet() {
        return std::make_pair<std::string, std::string>(dbName, setName);
    }

    int16_t whichType() {
        return typeID;
    }

    ENABLE_DEEP_COPY

private:
    String dbName;
    String setName;
    int16_t typeID;
};
}

#endif
