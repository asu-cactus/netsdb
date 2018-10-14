#ifndef DISTRIBUTEDSTORAGEREMOVEHASHSET_H
#define DISTRIBUTEDSTORAGEREMOVEHASHSET_H

// by Jia, Jul 2017

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "DataTypes.h"

// PRELOAD %DistributedStorageRemoveHashSet%

namespace pdb {

// encapsulates a request to remove a set in storage
class DistributedStorageRemoveHashSet : public Object {

public:
    DistributedStorageRemoveHashSet() {}
    ~DistributedStorageRemoveHashSet() {}

    DistributedStorageRemoveHashSet(std::string dataBase, std::string setName, SetType setType)
        : dataBase(dataBase), setName(setName), setType(setType) {}

    std::string getDatabase() {
        return dataBase;
    }

    std::string getSetName() {
        return setName;
    }

    SetType getSetType() {
        return setType;
    }

    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    SetType setType;
};
}

#endif  // DISTRIBUTEDSTORAGEREMOVEHASHSET_H
