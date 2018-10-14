#ifndef DISTRIBUTEDSTORAGEMANAGERREMOVEDATABASE_H
#define DISTRIBUTEDSTORAGEMANAGERREMOVEDATABASE_H

// PRELOAD %DistributedStorageRemoveDatabase%

namespace pdb {

// encapsulates a request to remove a database in the storage (and catalog)
class DistributedStorageRemoveDatabase : public Object {

public:
    DistributedStorageRemoveDatabase() {}
    ~DistributedStorageRemoveDatabase() {}

    DistributedStorageRemoveDatabase(std::string dataBase) : database(dataBase) {}

    std::string getDatabase() {
        return database;
    }

    ENABLE_DEEP_COPY

private:
    String database;
};
}

#endif  // DISTRIBUTEDSTORAGEMANAGERREMOVEDATABASE_H
