
#ifndef CAT_TYPE_SEARCH_RES_H
#define CAT_TYPE_SEARCH_RES_H

#include "Object.h"
#include "Handle.h"

// PRELOAD %CatTypeSearchResult%

namespace pdb {

// stores the result of searching for a type
class CatTypeSearchResult : public Object {

public:
    CatTypeSearchResult(){};
    ~CatTypeSearchResult(){};
    CatTypeSearchResult(int16_t searchRes) : findMe(searchRes) {}

    int16_t getTypeID() {
        return findMe;
    }

    ENABLE_DEEP_COPY

private:
    int16_t findMe;
};
}

#endif
