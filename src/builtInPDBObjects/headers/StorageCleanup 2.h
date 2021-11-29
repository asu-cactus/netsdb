
#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_CLEANUP_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_CLEANUP_H_


#include "Object.h"

//  PRELOAD %StorageCleanup%

namespace pdb {
// this object type is sent to the server to tell it there is no more page to load, scan finished at
// frontend
class StorageCleanup : public pdb::Object {

public:
    StorageCleanup(bool flushOrNot = true) {
        this->flushOrNot = flushOrNot;
    }
    ~StorageCleanup() {}

    bool isFlushing() {
        return flushOrNot;
    }

    ENABLE_DEEP_COPY

private:
    bool flushOrNot;
};
}

#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_CLEANUP_H_ */
