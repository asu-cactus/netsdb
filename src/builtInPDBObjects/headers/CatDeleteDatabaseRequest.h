
#ifndef CAT_DELETE_DB_H
#define CAT_DELETE_DB_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatDeleteDatabaseRequest%

namespace pdb {

// encapsulates a request to delete a Database
class CatDeleteDatabaseRequest : public Object {

public:
    ~CatDeleteDatabaseRequest() {}
    CatDeleteDatabaseRequest() {}
    CatDeleteDatabaseRequest(std::string dbName) : dbName(dbName) {}
    CatDeleteDatabaseRequest(const Handle<CatDeleteDatabaseRequest>& requestToCopy) {
        dbName = requestToCopy->dbName;
    }

    std::string dbToDelete() {
        return dbName;
    }

    ENABLE_DEEP_COPY

private:
    String dbName;
};
}

#endif
