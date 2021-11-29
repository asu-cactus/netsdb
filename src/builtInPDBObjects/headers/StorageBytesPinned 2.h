
#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_BYTESPINNED_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_BYTESPINNED_H_

#include <cstddef>

#include "Object.h"
#include "DataTypes.h"

//  PRELOAD %StorageBytesPinned%

namespace pdb {
// this object type is sent from the server to backend to tell it a page is pinned for it.
class StorageBytesPinned : public pdb::Object {


public:
    StorageBytesPinned() {}
    ~StorageBytesPinned() {}


    // get/set sizeOfBytes
    size_t getSizeOfBytes() {
        return this->sizeOfBytes;
    }
    void setSizeOfBytes(size_t sizeOfBytes) {
        this->sizeOfBytes = sizeOfBytes;
    }


    // get/set page offset in shared memory pool
    size_t getSharedMemOffset() {
        return this->sharedMemOffset;
    }
    void setSharedMemOffset(size_t offset) {
        this->sharedMemOffset = offset;
    }

    ENABLE_DEEP_COPY

private:
    size_t sizeOfBytes;
    size_t sharedMemOffset;
};
}

#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_BYTESPINNED_H_ */
