
#ifndef CAT_REG_TYPE_H
#define CAT_REG_TYPE_H

#include "Object.h"
#include "Handle.h"
#include "PDBVector.h"

// PRELOAD %CatRegisterType%

namespace pdb {

// encapsulates a request to regster a type in the catalog
class CatRegisterType : public Object {

public:
    CatRegisterType() {}
    ~CatRegisterType() {}

    ENABLE_DEEP_COPY
};
}

#endif
