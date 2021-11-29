
#ifndef PDBCM_STORAGEREMOVEDATABASE_H
#define PDBCM_STORAGEREMOVEDATABASE_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "DataTypes.h"

// PRELOAD %StorageRemoveDatabase%

namespace pdb {

// encapsulates a request to remove a database from storage
class StorageRemoveDatabase : public Object {

public:
    StorageRemoveDatabase() {}
    ~StorageRemoveDatabase() {}

    StorageRemoveDatabase(std::string dataBase) : database(dataBase) {}

    std::string getDatabase() {
        return database;
    }

    ENABLE_DEEP_COPY

private:
    String database;
};
}


#endif  // PDBCM_STORAGEREMOVEDATABASE_H
