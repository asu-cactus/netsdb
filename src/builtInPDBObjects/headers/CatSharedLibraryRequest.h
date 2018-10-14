
#ifndef CAT_SHARED_LIB_REQ_H
#define CAT_SHARED_LIB_REQ_H

#include "Object.h"
#include "Handle.h"

// PRELOAD %CatSharedLibraryRequest%

namespace pdb {

// encapsulates a request to obtain a shared library from the catalog
class CatSharedLibraryRequest : public Object {

public:
    CatSharedLibraryRequest(){};
    CatSharedLibraryRequest(int16_t searchForMe) : findMe(searchForMe) {}
    ~CatSharedLibraryRequest(){};

    int16_t getTypeID() {
        return findMe;
    }

    ENABLE_DEEP_COPY

private:
    int16_t findMe;
};
}

#endif
