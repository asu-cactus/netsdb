
#ifndef STORAGE_ADD_DATABASE_H
#define STORAGE_ADD_DATABASE_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageAddDatabase%

namespace pdb {

// encapsulates a request to add a database in the storage (and catalog)
class StorageAddDatabase : public Object {

public:
    StorageAddDatabase() {}
    ~StorageAddDatabase() {}

    StorageAddDatabase(std::string dataBase) : database(dataBase) {}

    std::string getDatabase() {
        return database;
    }

    ENABLE_DEEP_COPY

private:
    String database;
};
}

#endif
