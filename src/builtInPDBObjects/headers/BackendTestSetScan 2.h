
#ifndef BACKEND_TEST_SET_SCAN_H
#define BACKEND_TEST_SET_SCAN_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "DataTypes.h"

// PRELOAD %BackendTestSetScan%

namespace pdb {

// encapsulates a request to scan a set stored in the database
class BackendTestSetScan : public Object {

public:
    BackendTestSetScan(DatabaseID dbIdIn, UserTypeID typeIdIn, SetID setIdIn) {
        dbId = dbIdIn;
        typeId = typeIdIn;
        setId = setIdIn;
    }

    BackendTestSetScan() {}
    ~BackendTestSetScan() {}

    DatabaseID getDatabaseID() {
        return dbId;
    }

    UserTypeID getUserTypeID() {
        return typeId;
    }

    SetID getSetID() {
        return setId;
    }

    ENABLE_DEEP_COPY

private:
    // this is the database that we are computing over
    DatabaseID dbId;

    // this is the type that we are computing over
    UserTypeID typeId;

    // and the set
    SetID setId;
};
}

#endif
