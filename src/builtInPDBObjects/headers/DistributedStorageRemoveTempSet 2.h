#ifndef DISTRIBUTEDSTORAGEREMOVETEMPSET_H
#define DISTRIBUTEDSTORAGEREMOVETEMPSET_H

// by Jia, Mar 2017

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DistributedStorageRemoveTempSet%

namespace pdb {

// encapsulates a request to remove a set in storage
class DistributedStorageRemoveTempSet : public Object {

public:
    DistributedStorageRemoveTempSet() {}
    ~DistributedStorageRemoveTempSet() {}

    DistributedStorageRemoveTempSet(std::string dataBase, std::string setName, std::string typeName)
        : dataBase(dataBase), setName(setName), typeName(typeName) {}

    std::string getDatabase() {
        return dataBase;
    }

    std::string getSetName() {
        return setName;
    }

    std::string getTypeName() {
        return typeName;
    }

    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
};
}

#endif  // DISTRIBUTEDSTORAGEREMOVETEMPSET_H
