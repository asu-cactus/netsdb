
#ifndef STORAGE_REMOVE_TEMP_SET_H
#define STORAGE_REMOVE_TEMP_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "DataTypes.h"

// PRELOAD %StorageRemoveTempSet%

namespace pdb {

// encapsulates a request to remove a temp set from storage
class StorageRemoveTempSet : public Object {

public:
    StorageRemoveTempSet() {}
    ~StorageRemoveTempSet() {}

    StorageRemoveTempSet(SetID setId) : setId(setId) {}

    SetID getSetID() {
        return setId;
    }


    ENABLE_DEEP_COPY

private:
    SetID setId;
};
}

#endif
