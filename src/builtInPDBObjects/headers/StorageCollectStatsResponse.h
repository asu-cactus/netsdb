
#ifndef STORAGE_COLLECT_STATS_RESPONSE_H
#define STORAGE_COLLECT_STATS_RESPONSE_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "SetIdentifier.h"

// PRELOAD %StorageCollectStatsResponse%

namespace pdb {

// encapsulates a request to return all user set information
class StorageCollectStatsResponse : public Object {

public:
    StorageCollectStatsResponse() {}
    ~StorageCollectStatsResponse() {}

    Handle<Vector<Handle<SetIdentifier>>>& getStats() {
        return stats;
    }

    void setStats(Handle<Vector<Handle<SetIdentifier>>> stats) {
        this->stats = makeObject<Vector<Handle<SetIdentifier>>>();
        this->stats = stats;
    }

    ENABLE_DEEP_COPY

private:
    Handle<Vector<Handle<SetIdentifier>>> stats;
};
}

#endif
