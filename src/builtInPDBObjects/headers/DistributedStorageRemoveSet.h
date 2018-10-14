#ifndef DISTRIBUTEDSTORAGEREMOVESET_H
#define DISTRIBUTEDSTORAGEREMOVESET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DistributedStorageRemoveSet%

namespace pdb {

// encapsulates a request to remove a set in storage
class DistributedStorageRemoveSet : public Object {

public:
    DistributedStorageRemoveSet() {}
    ~DistributedStorageRemoveSet() {}

    DistributedStorageRemoveSet(std::string dataBase, std::string setName)
        : dataBase(dataBase), setName(setName) {}

    std::string getDatabase() {
        return dataBase;
    }

    std::string getSetName() {
        return setName;
    }

    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
};
}

#endif  // DISTRIBUTEDSTORAGEREMOVESET_H
