
#ifndef STORAGE_TEST_SET_COPY_H
#define STORAGE_TEST_SET_COPY_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include <string>

// PRELOAD %StorageTestSetCopy%

namespace pdb {

// encapsulates a request to copy a set stored in the database into another set
class StorageTestSetCopy : public Object {

public:
    StorageTestSetCopy(std::string dbNameIn,
                       std::string setNameIn,
                       std::string dbNameOut,
                       std::string setNameOut) {
        srcDatabaseName = dbNameIn;
        srcSetName = setNameIn;
        destDatabaseName = dbNameOut;
        destSetName = setNameOut;
    }

    StorageTestSetCopy() {}
    ~StorageTestSetCopy() {}

    std::string getDatabaseIn() {
        return srcDatabaseName;
    }

    std::string getSetNameIn() {
        return srcSetName;
    }

    std::string getDatabaseOut() {
        return destDatabaseName;
    }

    std::string getSetNameOut() {
        return destSetName;
    }

    ENABLE_DEEP_COPY

private:
    // this is the database name of the source set
    String srcDatabaseName;

    // this is the name of the source set
    String srcSetName;

    // this is the database name of the destination set
    String destDatabaseName;

    // this is the name of the destination set
    String destSetName;
};
}

#endif
