#ifndef FF_SINGLE_MATRIX_H
#define FF_SINGLE_MATRIX_H

#include "Object.h"
#include "Handle.h"
#include "FFMatrixBlock.h"

using namespace pdb;
class FFSingleMatrix : public Object {
private:
    int key = 1;  // Any constant should work here!
    FFMatrixBlock myValue;

public:
    ENABLE_DEEP_COPY

    ~FFSingleMatrix() {}
    FFSingleMatrix() {}

    FFSingleMatrix(FFMatrixBlock block) : key(1), myValue(block) {}

    size_t hash() const override{
        return myValue.hash();
    }

    int& getKey() {
        return key;
    }

    FFMatrixBlock& getValue() {
        return myValue;
    }

    void print() {
        std::cout << "FF Single Matrix" << std::endl; 
        myValue.print();
    }

};


#endif
