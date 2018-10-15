
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
                  size_t pageSize = DEFAULT_PAGE_SIZE,
                  size_t desiredSize = 1)
        : dataBase(dataBase), setName(setName), typeName(typeName), pageSize(pageSize), desiredSize(desiredSize) {}

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

    size_t getDesiredSize() {
        return desiredSize;
    }


    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
    size_t pageSize;
    size_t desiredSize;
};
}

#endif
