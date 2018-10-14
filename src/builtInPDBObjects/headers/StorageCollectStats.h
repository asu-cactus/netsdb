
#ifndef STORAGE_COLLECT_STATS_H
#define STORAGE_COLLECT_STATS_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageCollectStats%

namespace pdb {

// encapsulates a request to return all user set information
class StorageCollectStats : public Object {

public:
    StorageCollectStats() {}
    ~StorageCollectStats() {}


    ENABLE_DEEP_COPY
};
}

#endif
