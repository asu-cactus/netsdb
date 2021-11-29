
#ifndef STORAGE_ADD_TYPE_H
#define STORAGE_ADD_TYPE_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageAddType%

namespace pdb {

// encapsulates a request to add a type in the storage
class StorageAddType : public Object {

public:
    StorageAddType() {}
    ~StorageAddType() {}

    StorageAddType(std::string typeNameIn, unsigned int typeCodeIn) : typeName(typeNameIn) {
        typeCode = typeCodeIn;
    }

    std::string getTypeName() {
        return typeName;
    }

    unsigned int getTypeCode() {
        return typeCode;
    }

    ENABLE_DEEP_COPY

private:
    String typeName;
    unsigned int typeCode;
};
}

#endif
