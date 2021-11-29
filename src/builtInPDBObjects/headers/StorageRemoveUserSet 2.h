
#ifndef STORAGE_REMOVE_USER_SET_H
#define STORAGE_REMOVE_USER_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageRemoveUserSet%

namespace pdb {

// encapsulates a request to remove a set from storage
class StorageRemoveUserSet : public Object {

public:
    StorageRemoveUserSet() {}
    ~StorageRemoveUserSet() {}

    StorageRemoveUserSet(std::string dataBase, std::string setName, std::string typeName)
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

#endif
