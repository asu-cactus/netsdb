
#ifndef CAT_CREATE_DB_H
#define CAT_CREATE_DB_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatCreateDatabaseRequest%

namespace pdb {

// encapsulates a request to search for a type in the catalog
class CatCreateDatabaseRequest : public Object {

public:
    ~CatCreateDatabaseRequest() {}
    CatCreateDatabaseRequest() {}
    CatCreateDatabaseRequest(std::string dbName) : dbName(dbName) {}

    std::string dbToCreate() {
        return dbName;
    }

    ENABLE_DEEP_COPY

private:
    String dbName;
};
}

#endif
