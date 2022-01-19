#ifndef BACKEND_GET_SET_H
#define BACKEND_GET_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "DataTypes.h"

// PRELOAD %BackendGetSet%

namespace pdb {

// encapsulates a request to scan a set stored in the database
class BackendGetSet : public Object {

public:
    BackendGetSet(DatabaseID dbIdIn, UserTypeID typeIdIn, SetID setIdIn, string databaseNameIn, string setNameIn) {
        dbId = dbIdIn;
        typeId = typeIdIn;
        setId = setIdIn;
        databaseName = databaseNameIn;
        setName = setNameIn;
    }

    BackendGetSet() {}
    ~BackendGetSet() {}

    DatabaseID getDatabaseID() {
        return dbId;
    }

    UserTypeID getUserTypeID() {
        return typeId;
    }

    SetID getSetID() {
        return setId;
    }

    std::string getDatabaseName() {
        return databaseName;
    }

    std::string getSetName() {
        return setName;
    }

    ENABLE_DEEP_COPY

private:
    // this is the database that we are computing over
    DatabaseID dbId;

    // this is the type that we are computing over
    UserTypeID typeId;

    // and the set
    SetID setId;

    String databaseName;

    String setName;
};
}

#endif