
#ifndef ARRAY_CC
#define ARRAY_CC

#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <cstring>

#include "Handle.h"
#include "Object.h"
#include "InterfaceFunctions.h"

// Note: we need to write all operations in constructors, destructors, and assignment operators
// WITHOUT using
// the underlying type in any way (including assignment, initialization, destruction, size).
//
namespace pdb {

template <class TypeContained>
void Array<TypeContained>::setUpAndCopyFrom(void* target, void* source) const {

    new (target) Array<TypeContained>();
    Array<TypeContained>& fromMe = *((Array<TypeContained>*)source);
    Array<TypeContained>& toMe = *((Array<TypeContained>*)target);

    // copy the number of slots
    toMe.numSlots = fromMe.numSlots;
    toMe.usedSlots = fromMe.usedSlots;

    // copy the type info
    toMe.typeInfo = fromMe.typeInfo;
    if (toMe.typeInfo.getTypeCode() == 0) {
        std::cout << "Array::setUpAndCopyFrom: typeInfo=0 before getSizeOfConsitituentObject"
                  << std::endl;
    }
    // copy everything over... use memmove on primitive type, or properly do the assignment on
    // everything else
    char* newLoc = (char*)toMe.data;
    char* data = (char*)fromMe.data;

    // get the type code for our child from the RHS
    // get the size of the constituent object
    uint32_t dataSize = 0;
    if (fromMe.usedSlots > 0)
#ifdef DEBUG_DEEP_COPY
        PDB_COUT << "fromMe.usedSlots=" << fromMe.usedSlots << std::endl;
#endif
    dataSize = toMe.typeInfo.getSizeOfConstituentObject(data);

#ifdef DEBUG_DEEP_COPY
    PDB_COUT << "dataSize=" << dataSize << std::endl;
#endif

    if (!toMe.typeInfo.descendsFromObject()) {
        // in this case, we might have a fundmanetal type... regardless, just do a simple bitwise
        // copy
        memmove(newLoc, data, dataSize * fromMe.usedSlots);
    } else {
        for (uint32_t i = 0; i < fromMe.usedSlots; i++) {
            try {
                toMe.typeInfo.setUpAndCopyFromConstituentObject(newLoc + dataSize * i,
                                                                data + dataSize * i);
            } catch (NotEnoughSpace& n) {
                // JiaNote: handle exception here
                PDB_COUT << "exception in array copy" << std::endl;
                toMe.usedSlots = i;
                throw n;
            }
        }
    }
}

// just use setUpAndCopyFrom to do a deep copy... this assumes that we have enough space!!
template <class TypeContained>
Array<TypeContained>::Array(const Array& toMe) {
    setUpAndCopyFrom(this, &toMe);
}

template <class TypeContained>
Array<TypeContained>::Array(uint32_t numSlotsIn, uint32_t numUsedSlots) {
    typeInfo.setup<TypeContained>();
    usedSlots = numUsedSlots;
    numSlots = numSlotsIn;
    if (typeInfo.descendsFromObject()) {
        // std :: cout << "numUsedSlots=" << numUsedSlots << std :: endl;
        // run the constructor on each object
        // std :: cout << "Calling object constructor.\n";
        for (uint32_t i = 0; i < numUsedSlots; i++) {
            new ((void*)&(((TypeContained*)(data))[i])) TypeContained();
        }
    } else {
        // std :: cout << "numUsedSlots=" << numUsedSlots << std :: endl;
        // std :: cout << "sizeof(TypeContained)=" << sizeof(TypeContained) << std :: endl;
        // zero out everything
        // std :: cout << "Just b-zeroing everything.\n";
        bzero(data, numUsedSlots * sizeof(TypeContained));
    }
}
template <class TypeContained>
Array<TypeContained>::Array(uint32_t numSlotsIn) {
    typeInfo.setup<TypeContained>();
    usedSlots = 0;
    numSlots = numSlotsIn;
}

template <class TypeContained>
Handle<Array<TypeContained>> Array<TypeContained>::doubleSize() {
    return resize(numSlots * 2);
}

template <class TypeContained>
bool Array<TypeContained>::isFull() {
    typeInfo.setup<TypeContained>();
    return usedSlots == numSlots;
}

template <class TypeContained>
Array<TypeContained>::Array() {
    typeInfo.setup<TypeContained>();
    usedSlots = 0;
    numSlots = 0;
}

// Note: because this can be called by Object.deleteObject (), it must be written so as to not use
// TypeContained
template <class TypeContained>
Array<TypeContained>::~Array() {
    /*
    // do no work if the guys we store do not come from pdb :: Object
    if (!typeInfo.descendsFromObject())
        return;

    // loop through and explicitly call the destructor on everything
    char* location = (char*)data;

    // get the size of the constituent object
    uint32_t objSize = 0;
    if (usedSlots > 0) {
        objSize = typeInfo.getSizeOfConstituentObject(location);
    }
    // now, delete each of the objects in there, if we have got an object type
    for (uint32_t i = 0; i < usedSlots; i++) {
	std::cout << "i=" << i << std::endl;
        typeInfo.deleteConstituentObject(location + i * objSize);
    }
    */
}

template <class TypeContained>
Handle<Array<TypeContained>> Array<TypeContained>::resize(uint32_t howMany) {

    // allocate the new Array
    Handle<Array<TypeContained>> tempArray =
        makeObjectWithExtraStorage<Array<TypeContained>>(sizeof(TypeContained) * howMany, howMany);

    // copy everything over
    TypeContained* newLoc = (TypeContained*)(tempArray->data);

    uint32_t max = usedSlots;
    if (max < howMany)
        max = howMany;

    uint32_t min = usedSlots;
    if (min > howMany)
        min = howMany;

    tempArray->usedSlots = min;

    // JiaNote: deep copy may cause exception, and cause usedSlots inconsistent
    size_t myUsedSlots = usedSlots;

    for (uint32_t i = 0; i < min || i < myUsedSlots; i++) {

        if (i < min) {
            new ((void*)&(newLoc[i])) TypeContained();
            newLoc[i] = ((TypeContained*)(data))[i];
        } else if (i < myUsedSlots) {
            ((TypeContained*)(data))[i].~TypeContained();
            // JiaNote: we need make usedSlots consistent in cases of exception
            usedSlots--;
        }
    }

    // empty out this guy
    return tempArray;
}

template <class TypeContained>
TypeContained& Array<TypeContained>::getObj(uint32_t which) {
    return ((TypeContained*)(data))[which];
}

template <class TypeContained>
void Array<TypeContained>::assign(uint32_t which, const TypeContained& val) {
    if (which > usedSlots) {
        std::cout << "which: " << which << std::endl;
        std::cout << "usedSlots: " << usedSlots << std::endl;
        std::cerr << "Bad: you are writing past the end of the vector!\n";
        return;
    }
    ((TypeContained*)(data))[which] = val;
}

template <class TypeContained>
void Array<TypeContained>::push_back(const TypeContained& val) {
    // need a placement new to correctly initialize before the copy
    new ((void*)&(((TypeContained*)(data))[usedSlots])) TypeContained();
    ((((TypeContained*)(data))[usedSlots])) = val;
    usedSlots++;
}


template <class TypeContained>
void Array<TypeContained>::push_back() {

    // need a placement new
    new ((void*)&(((TypeContained*)(data))[usedSlots])) TypeContained();
    usedSlots++;
}


template <class TypeContained>
TypeContained* Array<TypeContained>::c_ptr() {
    return ((TypeContained*)(data));
}

template <class TypeContained>
void Array<TypeContained>::pop_back() {
    if (usedSlots != 0) {
        ((TypeContained*)(data))[usedSlots - 1].~TypeContained();
        usedSlots--;
    }
}

template <class TypeContained>
uint32_t Array<TypeContained>::numUsedSlots() {
    return usedSlots;
}

template <class TypeContained>
void Array<TypeContained>::setUsed(uint32_t toMe) {
    usedSlots = toMe;
}

template <class TypeContained>
void Array<TypeContained>::deleteObject(void* deleteMe) {
    deleter(deleteMe, this);
}

template <class TypeContained>
size_t Array<TypeContained>::getSize(void* forMe) {
    if (((Array<TypeContained>*)forMe)->typeInfo.getTypeCode() == 0) {
        std::cout << "Array::getSize: typeInfo = 0 before getSizeOfConstituent" << std::endl;
    }
    return sizeof(Array<TypeContained>) +
        ((Array<TypeContained>*)forMe)->typeInfo.getSizeOfConstituentObject(forMe) *
        ((Array<TypeContained>*)forMe)->numSlots;
}
}

#endif
