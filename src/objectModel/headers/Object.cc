
#ifndef OBJECT_CC
#define OBJECT_CC

#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>

namespace pdb {

// empty placement new
inline void* Object::operator new(std::size_t count, void* ptr) {
    return ptr;
}

inline void* Object::operator new(size_t sz, const std::nothrow_t& tag) {

    std::cerr << "This should not be called!!  We are in overloaded new.\n";
    std::cerr << "Instead, a placement new should be done by the handle.\n";
    exit(1);
}

inline void* Object::operator new(size_t sz) {

    std::cerr << "This should not be called!!  We are in overloaded new.\n";
    std::cerr << "Instead, a placement new should be done by the handle.\n";
    exit(1);
}

inline void Object::operator delete(void* me) {

    std::cerr << "This should not be called!!  We are in overloaded delete.\n";
    std::cerr << "Instead, the deletion should be done by the handle.\n";
    exit(1);
}

inline void Object::setVTablePtr(void* setToMe) {
    // This assumes that the vTable pointer is located at the very beginning of the object.
    // This has been verified on several compilers.

    // this will NOT set the vTable pointer to be a null pointer
    if (setToMe == nullptr)
        return;

    int** temp = (int**)this;
    *temp = (int*)setToMe;
}

inline void* Object::getVTablePtr() {
    int** temp = (int**)this;
    return *temp;
}

inline size_t Object::hash() const {
    return (size_t)(this)%128;
}


inline void Object::setUpAndCopyFrom(void* target, void* source) const {
    std::cerr
        << "Bad: you are trying to do a deep Object copy without the ENABLE_DEEP_COPY macro.\n";
    int* a = 0;
    *a = 12;
    exit(1);
}

inline void Object::deleteObject(void* deleteMe) {
    std::cerr
        << "Bad: you are trying to do a deep Object copy without the ENABLE_DEEP_COPY macro.\n";
    int* a = 0;
    *a = 12;
    exit(1);
}

inline size_t Object::getSize(void* forMe) {
    std::cerr
        << "Bad: you are trying to do a deep Object copy without the ENABLE_DEEP_COPY macro.\n";
    int* a = 0;
    *a = 12;
    exit(1);
}
}

#endif
