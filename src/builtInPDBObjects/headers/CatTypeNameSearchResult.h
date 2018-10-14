
#ifndef CAT_TYPE_SEARCH_REQ_H
#define CAT_TYPE_SEARCH_REQ_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %CatTypeNameSearchResult%

namespace pdb {

// encapsulates a request to obtain a type name from the catalog
class CatTypeNameSearchResult : public Object {

public:
    CatTypeNameSearchResult(){};
    ~CatTypeNameSearchResult(){};
    CatTypeNameSearchResult(std::string typeName, bool success, std::string errMsg)
        : typeName(typeName), errMsg(errMsg), success(success) {}

    std::string getTypeName() {
        return typeName;
    }

    std::pair<bool, std::string> wasSuccessful() {
        return std::make_pair(success, errMsg);
    }

    ENABLE_DEEP_COPY

private:
    String typeName;
    String errMsg;
    bool success;
};
}

#endif
