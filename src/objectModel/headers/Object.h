
#ifndef OBJECT_H
#define OBJECT_H

#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>

#include "TypeName.h"
#include "DeepCopy.h"

namespace pdb {

enum ObjectPolicy { defaultObject, noReferenceCountObject };


// This is the basic PDB Object type.  Everything that is stored in PDB should
// derive from this.  Currently, it simply overrides all of the new () operators
// so that it is basically impossible to create an object via new ().  Additional
// v-table fixing code will be added in the near future.
//
class Object {

public:
    // these all just error out since they should never be called
    static void* operator new(size_t sz, const std::nothrow_t& tag);
    static void* operator new(size_t sz);
    static void operator delete(void* me);
    static void* operator new(std::size_t count, void* ptr);
    void setVTablePtr(void* setToMe);
    void* getVTablePtr();
    virtual size_t hash() const {
       
       size_t x = (size_t)(this);
       //std::cout << "this:" << x << std::endl;
       x = ((x >> 16) ^ x) * 0x45d9f3b;
       x = ((x >> 16) ^ x) * 0x45d9f3b;
       x = (x >> 16) ^ x;
       return x;
    };

    // these are properly defined via the ENABLE_DEEP_COPY macro
    virtual void setUpAndCopyFrom(void* target, void* source) const;
    virtual void deleteObject(void* deleteMe);
    virtual size_t getSize(void* ofMe);
    virtual std::string toString() {return "";};
};
}

#include "Object.cc"

#endif
