
#ifndef STORAGE_REMOVE_HASH_SET_H
#define STORAGE_REMOVE_HASH_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "DataTypes.h"

// PRELOAD %StorageRemoveHashSet%

namespace pdb {

// encapsulates a request to remove a hash set from storage
class StorageRemoveHashSet : public Object {

public:
    StorageRemoveHashSet() {}
    ~StorageRemoveHashSet() {}

    StorageRemoveHashSet(std::string dataBase, std::string setName, SetType setType)
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

#endif
