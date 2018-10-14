
#ifndef STORAGE_TEST_SET_SCAN_H
#define STORAGE_TEST_SET_SCAN_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include <string>

// PRELOAD %StorageTestSetScan%

namespace pdb {

// encapsulates a request to scan a set stored in the database
class StorageTestSetScan : public Object {

public:
    StorageTestSetScan(std::string dbNameIn, std::string setNameIn) {
        dbName = dbNameIn;
        setName = setNameIn;
    }

    StorageTestSetScan() {}
    ~StorageTestSetScan() {}

    std::string getDatabase() {
        return dbName;
    }

    std::string getSetName() {
        return setName;
    }

    ENABLE_DEEP_COPY

private:
    // this is the database that we are computing over
    String dbName;

    // and the set
    String setName;
};
}

#endif
