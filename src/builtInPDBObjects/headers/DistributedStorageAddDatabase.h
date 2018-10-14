#ifndef DISTRIBUTEDSTORAGEADDDATABASE_H
#define DISTRIBUTEDSTORAGEADDDATABASE_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DistributedStorageAddDatabase%

namespace pdb {

// encapsulates a request to add a database in the storage (and catalog)
class DistributedStorageAddDatabase : public Object {

public:
    DistributedStorageAddDatabase() {}
    ~DistributedStorageAddDatabase() {}

    DistributedStorageAddDatabase(std::string dataBase) : database(dataBase) {}

    std::string getDatabase() {
        return database;
    }

    ENABLE_DEEP_COPY

private:
    String database;
};
}

#endif  // DISTRIBUTEDSTORAGEADDDATABASE_H
