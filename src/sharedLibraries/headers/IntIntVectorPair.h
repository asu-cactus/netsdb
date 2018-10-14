#ifndef INT_INT_VECTOR_PAIR_H
#define INT_INT_VECTOR_PAIR_H

#include "Object.h"
#include "PDBVector.h"
#include "Handle.h"

/* This class is for storing a (int, int vector) pair */
using namespace pdb;
class IntIntVectorPair : public Object {

private:
    int myInt;
    Vector<int> myVector;

public:
    ENABLE_DEEP_COPY

    ~IntIntVectorPair() {}
    IntIntVectorPair() {}

    IntIntVectorPair(int fromInt, Handle<Vector<int>>& fromVector) {
        this->myVector = *fromVector;
        this->myInt = fromInt;
    }

    int getInt() {
        return this->myInt;
    }

    Vector<int>& getVector() {
        return this->myVector;
    }

    int& getKey() {
        return this->myInt;
    }

    Vector<int>& getValue() {
        return this->myVector;
    }
};

/* Overload the + operator */
namespace pdb {
inline Vector<int>& operator+(Vector<int>& lhs, Vector<int>& rhs) {
    int size = lhs.size();
    if (size != rhs.size()) {
        std::cout << "You cannot add two vectors in different sizes!" << std::endl;
        return lhs;
    }
    int* lhsArray = lhs.c_ptr();
    int* rhsArray = rhs.c_ptr();
    for (int i = 0; i < size; ++i) {
        lhsArray[i] = lhsArray[i] + rhsArray[i];
    }
    return lhs;
}
}
#endif
