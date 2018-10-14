
#ifndef SCAN_SUPERVISOR_SET_H
#define SCAN_SUPERVISOR_SET_H

#include "ScanUserSet.h"
#include "Supervisor.h"

using namespace pdb;
class ScanSupervisorSet : public ScanUserSet<Supervisor> {

public:
    ENABLE_DEEP_COPY

    ScanSupervisorSet() {}

    ScanSupervisorSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
