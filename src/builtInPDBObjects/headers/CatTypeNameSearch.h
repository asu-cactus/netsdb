
#ifndef CATTYPENAMESEARCH_H
#define CATTYPENAMESEARCH_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatTypeNameSearch%

namespace pdb {

// encapsulates a request to search for a type in the catalog
class CatTypeNameSearch : public Object {

public:
    CatTypeNameSearch() {}
    ~CatTypeNameSearch() {}

    CatTypeNameSearch(std::string searchForMe) : objectName(searchForMe) {}

    std::string getObjectTypeName() {
        return objectName;
    }

    ENABLE_DEEP_COPY

private:
    String objectName;
};
}

#endif
