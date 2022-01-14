
#ifndef SIMPLE_REQ_RES_H
#define SIMPLE_REQ_RES_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include <utility>

// PRELOAD %SimpleRequestResult%

namespace pdb {

// encapsulates a request to obtain a shared library from the catalog
class SimpleRequestResult : public Object {

public:
    SimpleRequestResult() {}
    ~SimpleRequestResult() {}

    // generally res should be true on success
    SimpleRequestResult(bool res, std::string errMsg) : res(res), errMsg(errMsg) {}

    ENABLE_DEEP_COPY

    void setNumHashKeys (int numHashKeys) {
        this->numHashKeys = numHashKeys;
    }

    int getNumHashKeys() {
        return numHashKeys;
    }

    std::pair<bool, std::string> getRes() {
        return std::make_pair(res, errMsg);
    }
    
private:
    bool res;
    String errMsg;
    int numHashKeys = 0;
};
}

#endif
