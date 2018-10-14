#ifndef LA_MAX_ELEMENT_OUTPUT_H
#define LA_MAX_ELEMENT_OUTPUT_H

#include "Object.h"
#include "Handle.h"
#include "LAMaxElementValueType.h"


// By Binhang, May 2017

// This class is designed for finding the maximal element by aggregation;
using namespace pdb;
class LAMaxElementOutputType : public Object {
private:
    int key = 1;  // Any constant should work here!
    LAMaxElementValueType myValue;

public:
    ENABLE_DEEP_COPY

    ~LAMaxElementOutputType() {}
    LAMaxElementOutputType() {}

    int& getKey() {
        return key;
    }

    LAMaxElementValueType& getValue() {
        return myValue;
    }

    void print() {
        std::cout << "LAMaxElementOutputType: value: " << myValue.getValue() << " index:("
                  << myValue.getRowIndex() << "," << myValue.getColIndex() << ")." << std::endl;
    }
};

#endif