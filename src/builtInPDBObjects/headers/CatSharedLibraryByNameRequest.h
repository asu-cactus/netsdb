
#ifndef CAT_SHARED_LIB_BY_NAME_REQ_H
#define CAT_SHARED_LIB_BY_NAME_REQ_H

#include "Object.h"
#include "Handle.h"

// PRELOAD %CatSharedLibraryByNameRequest%

namespace pdb {

// encapsulates a request to obtain a shared library from the catalog
// given the name of the type, this is used when the typeId is unknown
class CatSharedLibraryByNameRequest : public Object {

public:
    CatSharedLibraryByNameRequest(){};

    CatSharedLibraryByNameRequest(int16_t identifierIn, String NameIn)
        : identifier(identifierIn), objectTypeName(NameIn) {}

    CatSharedLibraryByNameRequest(const CatSharedLibraryByNameRequest& objectToCopy) {
        objectTypeName = objectToCopy.objectTypeName;
        identifier = objectToCopy.identifier;
    }

    ~CatSharedLibraryByNameRequest(){};

    String getTypeLibraryName() {
        return objectTypeName;
    }

    int16_t getTypeLibraryId() {
        return identifier;
    }

    ENABLE_DEEP_COPY

private:
    int16_t identifier;
    String objectTypeName;
};
}

#endif
