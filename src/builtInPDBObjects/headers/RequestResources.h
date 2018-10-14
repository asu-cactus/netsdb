
#ifndef REQUEST_RESOURCES_H
#define REQUEST_RESOURCES_H

#include "Object.h"
#include "Handle.h"

// PRELOAD %RequestResources%

namespace pdb {

// encapsulates a request to request resources for running a query
class RequestResources : public Object {

public:
    RequestResources() {}
    ~RequestResources() {}

    RequestResources(int numCores, int memSize) : numCores(numCores), memSize(memSize) {}

    int getNumCores() {
        return numCores;
    }

    int getMemSize() {
        return memSize;
    }

    ENABLE_DEEP_COPY

private:
    // number of CPU cores
    int numCores;

    // total size of memory (in MB)
    int memSize;
};
}

#endif
