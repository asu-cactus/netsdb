
#include "Object.h"
#include "PDBTemplateBase.h"
#include "Handle.h"
#include "Array.h"

#ifndef HOLDER_H
#define HOLDER_H

#include <cstddef>
#include <iostream>
#include <iterator>
#include <cstring>

// PRELOAD %Holder<Nothing>%

namespace pdb {


template <class TargetType>
class Holder : public Object {


public:
    ENABLE_DEEP_COPY

    Handle<TargetType> child;
};
}


#endif
