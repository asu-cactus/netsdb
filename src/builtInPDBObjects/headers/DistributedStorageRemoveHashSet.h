#ifndef DISTRIBUTEDSTORAGEREMOVEHASHSET_H
#define DISTRIBUTEDSTORAGEREMOVEHASHSET_H

// by Jia, Jul 2017

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DistributedStorageRemoveHashSet%

namespace pdb {

// encapsulates a request to remove a set in storage
class DistributedStorageRemoveHashSet : public Object {

public:
    DistributedStorageRemoveHashSet() {}
    ~DistributedStorageRemoveHashSet() {}

    DistributedStorageRemoveHashSet(std::string hashSetName)
        : hashSetName(hashSetName) {}

    std::string getHashSetName() {
        return hashSetName;
    }

    


    ENABLE_DEEP_COPY

private:
    String hashSetName;
};
}

#endif  // DISTRIBUTEDSTORAGEREMOVEHASHSET_H
