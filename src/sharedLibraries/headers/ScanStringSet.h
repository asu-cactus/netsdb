
#ifndef SCAN_STRING_SET_H
#define SCAN_STRING_SET_H

#include "ScanUserSet.h"
#include "PDBString.h"

using namespace pdb;
class ScanStringSet : public ScanUserSet<String> {

public:
    ENABLE_DEEP_COPY

    ScanStringSet() {}

    ScanStringSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
