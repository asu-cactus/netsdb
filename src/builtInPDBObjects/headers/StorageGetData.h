
#ifndef STORAGE_Get_DATA_H
#define STORAGE_Get_DATA_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageGetData%

namespace pdb {

// encapsulates a request to get data from a set in storage
class StorageGetData : public Object {

public:
    StorageGetData() {}
    ~StorageGetData() {}

    StorageGetData(std::string dataBase, std::string setName, std::string typeName)
        : dataBase(dataBase), setName(setName), typeName(typeName) {}

    std::string getDatabase() {
        return dataBase;
    }

    std::string getSetName() {
        return setName;
    }

    std::string getType() {
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
