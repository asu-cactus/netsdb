
#ifndef STORAGE_ADD_SET_H
#define STORAGE_ADD_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "Configuration.h"

// PRELOAD %StorageAddSet%

namespace pdb {

// encapsulates a request to add a set in storage
class StorageAddSet : public Object {

public:
    StorageAddSet() {}
    ~StorageAddSet() {}

    StorageAddSet(std::string dataBase,
                  std::string setName,
                  std::string typeName,
                  size_t pageSize = DEFAULT_PAGE_SIZE)
        : dataBase(dataBase), setName(setName), typeName(typeName), pageSize(pageSize) {}

    std::string getDatabase() {
        return dataBase;
    }

    std::string getSetName() {
        return setName;
    }

    std::string getTypeName() {
        return typeName;
    }

    size_t getPageSize() {
        return pageSize;
    }


    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
    size_t pageSize;
};
}

#endif
