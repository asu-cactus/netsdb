
#ifndef SCAN_OPTIMIZED_SUPERVISOR_SET_H
#define SCAN_OPTIMIZED_SUPERVISOR_SET_H

#include "ScanUserSet.h"
#include "OptimizedSupervisor.h"

using namespace pdb;
class ScanOptimizedSupervisorSet : public ScanUserSet<OptimizedSupervisor> {

public:
    ENABLE_DEEP_COPY

    ScanOptimizedSupervisorSet() {}

    ScanOptimizedSupervisorSet(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
