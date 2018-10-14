#ifndef DOUBLE_VECTOR_RESULT_H
#define DOUBLE_VECTOR_RESULT_H


#include "Object.h"
#include "DoubleVector.h"
// PRELOAD %DoubleVectorResult%

namespace pdb {

class DoubleVectorResult : public Object {

public:
    DoubleVectorResult() {}


    DoubleVector doubleVector;
    int identifier;

    ENABLE_DEEP_COPY

    void print() {
        this->doubleVector.print();
    }

    int& getKey() {
        return identifier;
    }

    DoubleVector& getValue() {
        return doubleVector;
    }
};
}


#endif
