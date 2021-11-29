
#ifndef STORAGE_ADD_OBJECT_IN_LOOP_H
#define STORAGE_ADD_OBJECT_IN_LOOP_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageAddObjectInLoop%

namespace pdb {

// encapsulates a request to add data to a set in storage
class StorageAddObjectInLoop : public Object {

public:
    StorageAddObjectInLoop() {
        loopEnded = false;
    }
    ~StorageAddObjectInLoop() {}

    StorageAddObjectInLoop(std::string dataBase,
                           std::string setName,
                           std::string typeName,
                           bool typeCheck = true)
        : dataBase(dataBase), setName(setName), typeName(typeName) {
        this->typeCheck = typeCheck;
        this->typeID = -1;
        loopEnded = false;
    }


    StorageAddObjectInLoop(std::string dataBase,
                           std::string setName,
                           std::string typeName,
                           int typeID,
                           bool typeCheck = true)
        : dataBase(dataBase), setName(setName), typeName(typeName) {
        this->typeCheck = typeCheck;
        this->typeID = typeID;
        loopEnded = false;
    }

    std::string getDatabase() {
        return dataBase;
    }

    std::string getSetName() {
        return setName;
    }

    std::string getType() {
        return typeName;
    }

    int getTypeID() {
        return typeID;
    }

    bool isTypeCheck() {
        return typeCheck;
    }

    void setLoopEnded() {
        loopEnded = true;
    }


    bool isLoopEnded() {
        return loopEnded;
    }

    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
    int typeID;
    bool typeCheck;
    bool loopEnded = false;
};
}

#endif
