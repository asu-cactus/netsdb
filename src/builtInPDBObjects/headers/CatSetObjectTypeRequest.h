
#ifndef CAT_SET_OBJ_TYPE_REQ_H
#define CAT_SET_OBJ_TYPE_REQ_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatSetObjectTypeRequest%

namespace pdb {

// encapsulates a request to obtain a shared library from the catalog
class CatSetObjectTypeRequest : public Object {

public:
    CatSetObjectTypeRequest(){};
    CatSetObjectTypeRequest(std::string dbNameIn, std::string setNameIn)
        : databaseName(dbNameIn), setName(setNameIn) {}
    ~CatSetObjectTypeRequest(){};

    std::string getDatabaseName() {
        return databaseName;
    }

    std::string getSetName() {
        return setName;
    }

    ENABLE_DEEP_COPY

private:
    String databaseName;
    String setName;
};
}

#endif
