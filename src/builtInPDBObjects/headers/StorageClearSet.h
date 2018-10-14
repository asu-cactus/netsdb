
#ifndef STORAGE_CLEAR_SET_H
#define STORAGE_CLEAR_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageClearSet%

namespace pdb {

// encapsulates a request to add a set in storage
class StorageClearSet : public Object {

public:
    StorageClearSet() {}
    ~StorageClearSet() {}

    StorageClearSet(std::string dataBase, std::string setName, std::string typeName)
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
