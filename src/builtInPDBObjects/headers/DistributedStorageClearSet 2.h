#ifndef DISTRIBUTEDSTORAGERECLEARSET_H
#define DISTRIBUTEDSTORAGERECLEARSET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DistributedStorageClearSet%

namespace pdb {

// encapsulates a request to add a set in storage
class DistributedStorageClearSet : public Object {

public:
    DistributedStorageClearSet() {}
    ~DistributedStorageClearSet() {}

    DistributedStorageClearSet(std::string dataBase, std::string setName, std::string typeName)
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

#endif  // DISTRIBUTEDSTORAGECLEARSET_H
