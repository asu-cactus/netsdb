
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

    StorageRemoveHashSet(std::string hashSetName)
        : hashSetName(hashSetName) {}


    std::string getHashSetName() {
        return hashSetName;
    }


    ENABLE_DEEP_COPY

private:
    String hashSetName;
};
}

#endif
