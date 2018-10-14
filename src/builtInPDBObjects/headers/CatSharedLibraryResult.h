
#ifndef CAT_SHARED_LIB_RES_H
#define CAT_SHARED_LIB_RES_H

#include "Object.h"
#include "Handle.h"
#include "PDBVector.h"
#include <utility>

// PRELOAD %CatSharedLibraryResult%

namespace pdb {

// encapsulates a shared library set from the catalog
class CatSharedLibraryResult : public Object {

public:
    CatSharedLibraryResult() {}
    CatSharedLibraryResult(bool res, std::string errMsg) : res(res), errMsg(errMsg) {}
    ~CatSharedLibraryResult() {}

    // this is used to actually store the shared library
    // will be loaded when sendBytes () is called...
    Vector<char> dataToSend;

    ENABLE_DEEP_COPY

    std::pair<bool, std::string> getRes() {
        return std::make_pair(res, errMsg);
    }

private:
    bool res;
    String errMsg;
};
}

#endif
