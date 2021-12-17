
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
                  size_t desiredSize = 1,
                  bool isMRU = true,
                  bool isTransient = true,
		  bool isSharedTensorBlockSet = false)
        : dataBase(dataBase), setName(setName), typeName(typeName), pageSize(pageSize), desiredSize(desiredSize), isMRU(isMRU), isTransient(isTransient), isSharedTensorBlockSet(isSharedTensorBlockSet) {}

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

    bool getMRUorNot() {
        return isMRU;
    }

    bool getTransientOrNot() {
        return isTransient;
    }

    bool getSharedTensorBlockSet() {
        return this->isSharedTensorBlockSet;
    }

    void setSharedTensorBlockSet(bool isSharedTensorBlockSet) {
        this->isSharedTensorBlockSet = isSharedTensorBlockSet;
    }

    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
    size_t pageSize;
    size_t desiredSize;
    bool isMRU;
    bool isTransient;
    bool isSharedTensorBlockSet;
};
}

#endif
