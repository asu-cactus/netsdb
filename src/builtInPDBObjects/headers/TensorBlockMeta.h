
#ifndef TENSOR_BLOCK_META_H
#define TENSOR_BLOCK_META_H


#include "Object.h"
#include "Handle.h"
#include "DataTypes.h"
#include "PDBVector.h"

// PRELOAD %TensorBlockMeta%

namespace pdb {

// encapsulates a request to add a set in storage
class TensorBlockMeta : public Object {

public:
    ENABLE_DEEP_COPY

    TensorBlockMeta() {}
    ~TensorBlockMeta() {}

    TensorBlockMeta(std::vector<int> dimensionIndexes) {
        this->dimensions = makeObject<Vector<int>>();
	for (int dim : dimensionIndexes) {
	    this->dimensions->push_back(dim);
	}
    }

    void print() {
	std::cout << "[ ";
        for (int i = 0; i < dimensions->size(); i++) {
	    std::cout << (*dimensions)[i] << " ";
	}
	std::cout << "]" << std::endl;
    }

    int getNumDimensions() {
        return dimensions->size();
    }

    Handle<Vector<int>> getDimensions() {
        return dimensions;
    }

    size_t hash() const override {
    
        size_t value = 0;
	for (int i = 0; i < dimensions->size(); i++) {
	    value = value + size_t((*dimensions)[i])*size_t(1000)*size_t(pow((size_t)(2), (size_t)(i)));
	}
	return value;	

    }

private:
    Handle<Vector<int>> dimensions=nullptr;
};
}

#endif
