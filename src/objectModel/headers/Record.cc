
#ifndef RECORD_CC
#define RECORD_CC

#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>

#include "Record.h"

namespace pdb {

#define CHAR_PTR(c) ((char*)c)

template <class ObjType>
size_t Record<ObjType>::numBytes() {
    return *((size_t*)this);
}

template <class ObjType>
size_t Record<ObjType>::rootObjectOffset() {
    return *((size_t*)(CHAR_PTR(this) + sizeof(size_t)));
}

template <class ObjType>
Handle<ObjType> Record<ObjType>::getRootObject() {
    PDB_COUT << "getRootObject: object offset=" << rootObjectOffset() << std::endl;
    // this is the location of the root object
    char* objLoc = CHAR_PTR(this) + rootObjectOffset();

    // and return it
    Handle<ObjType> returnVal;
    returnVal.offset = objLoc - CHAR_PTR(&(returnVal));
    return returnVal;
}
}
#endif
