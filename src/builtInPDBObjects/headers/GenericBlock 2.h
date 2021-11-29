#ifndef GENERIC_BLOCK_H
#define GENERIC_BLOCK_H

// by Jia, Oct, 2016

#include "PDBVector.h"
#include "Object.h"

//  PRELOAD %GenericBlock%
namespace pdb {

// this class encapsulates a block of tuples/objects
// a page in user set can be transformed into a vector of generic blocks
// a generic block will be the basic unit of execution in pipeline
// most processors are based on generic block, except two: bundle processor and unbundle processor
// a bundle processor converts several pages into a vector of generic blocks;
// an unbundle processor converts a vector of generic blocks into several pages;

class GenericBlock : public Object {

private:
    Vector<Handle<Object>> block;

public:
    ENABLE_DEEP_COPY

    ~GenericBlock() {}

    GenericBlock() {}
    GenericBlock(size_t batchSize) : block(batchSize) {}
    Vector<Handle<Object>>& getBlock() {
        return block;
    }
};
}

#endif
