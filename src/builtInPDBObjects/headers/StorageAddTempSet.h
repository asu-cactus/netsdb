
#ifndef STORAGE_ADD_TEMP_SET_H
#define STORAGE_ADD_TEMP_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "Configuration.h"

// PRELOAD %StorageAddTempSet%

namespace pdb {

// encapsulates a request to add a set in storage
class StorageAddTempSet : public Object {

public:
    StorageAddTempSet() {}
    ~StorageAddTempSet() {}

    StorageAddTempSet(std::string setName, size_t pageSize = DEFAULT_PAGE_SIZE)
        : setName(setName), pageSize(pageSize) {}

    std::string getSetName() {
        return setName;
    }

    size_t getPageSize() {
        return pageSize;
    }

    ENABLE_DEEP_COPY

private:
    String setName;
    size_t pageSize;
};
}

#endif
