
#ifndef CAT_DELETE_SET_H
#define CAT_DELETE_SET_H

#include "Object.h"
#include "PDBString.h"
#include "Handle.h"

// PRELOAD %CatDeleteSetRequest%

namespace pdb {

// encapsulates a request to delete a set
class CatDeleteSetRequest : public Object {

public:
    ~CatDeleteSetRequest() {}
    CatDeleteSetRequest() {}
    CatDeleteSetRequest(std::string dbName, std::string setName)
        : dbName(dbName), setName(setName) {}

    CatDeleteSetRequest(const Handle<CatDeleteSetRequest>& requestToCopy) {
        setName = requestToCopy->setName;
        dbName = requestToCopy->dbName;
    }

    std::pair<std::string, std::string> whichSet() {
        return std::make_pair<std::string, std::string>(dbName, setName);
    }

    ENABLE_DEEP_COPY

private:
    String dbName;
    String setName;
};
}

#endif
