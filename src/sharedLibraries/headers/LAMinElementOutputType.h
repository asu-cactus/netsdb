#ifndef LA_MIN_ELEMENT_OUTPUT_H
#define LA_MIN_ELEMENT_OUTPUT_H

#include "Object.h"
#include "Handle.h"
#include "LAMinElementValueType.h"


// By Binhang, May 2017

// This class is first designed for finding the minimal element by aggregation, but it can be also
// applied for any other purpose;
using namespace pdb;
class LAMinElementOutputType : public Object {
private:
    int key = 1;  // Any constant should work here!
    LAMinElementValueType myValue;

public:
    ENABLE_DEEP_COPY

    ~LAMinElementOutputType() {}
    LAMinElementOutputType() {}

    int& getKey() {
        return key;
    }

    LAMinElementValueType& getValue() {
        return myValue;
    }

    void print() {
        std::cout << "LAMinElementOutputType: value: " << myValue.getValue() << " index:("
                  << myValue.getRowIndex() << "," << myValue.getColIndex() << ")." << std::endl;
    }
};

#endif