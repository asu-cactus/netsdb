
#ifndef STORAGE_ADD_DATA_H
#define STORAGE_ADD_DATA_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageAddData%

namespace pdb {

// encapsulates a request to add data to a set in storage
class StorageAddData : public Object {

public:
    StorageAddData() {}
    ~StorageAddData() {}

    StorageAddData(std::string dataBase,
                   std::string setName,
                   std::string typeName,
                   bool typeCheck = true,
                   bool flushOrNot = true,
                   bool compressedOrNot = false,
                   bool directPutOrNot = false)
        : dataBase(dataBase), setName(setName), typeName(typeName) {
        this->typeCheck = typeCheck;
        this->typeID = -1;
        this->flushOrNot = flushOrNot;
        this->compressedOrNot = compressedOrNot;
        this->directPutOrNot = directPutOrNot;
    }


    StorageAddData(std::string dataBase,
                   std::string setName,
                   std::string typeName,
                   int typeID,
                   bool typeCheck = true,
                   bool flushOrNot = true,
                   bool compressedOrNot = false,
                   bool directPutOrNot = false)
        : dataBase(dataBase), setName(setName), typeName(typeName) {
        this->typeCheck = typeCheck;
        this->typeID = typeID;
        this->flushOrNot = flushOrNot;
        this->compressedOrNot = compressedOrNot;
        this->directPutOrNot = directPutOrNot;
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

    bool isFlushing() {
        return flushOrNot;
    }

    bool isCompressed() {
        return compressedOrNot;
    }

    bool isDirectPut() {
        return directPutOrNot;
    }

    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
    int typeID;
    bool typeCheck;
    bool flushOrNot;
    bool compressedOrNot;
    bool directPutOrNot;
};
}

#endif
