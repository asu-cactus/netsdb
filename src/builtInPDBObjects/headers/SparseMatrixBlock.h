#ifndef SPARSEMATRIXBLOCK_H
#define SPARSEMATRIXBLOCK_H

#include "Handle.h"
#include "PDBVector.h"

// PRELOAD %SparseMatrixBlock%

namespace pdb {

class SparseMatrixBlock : public pdb::Object {
    ENABLE_DEEP_COPY
    // Type alias
    using MapBlock = pdb::Vector<pdb::Handle<pdb::Map<int, float>>>;

  public:
    int blockID;
    int blockSize;
    pdb::Handle<MapBlock> mapBlockHandle;

    SparseMatrixBlock() {}
    ~SparseMatrixBlock() {}
    SparseMatrixBlock(int blockid, int blocksize)
        : blockID{blockid}, blockSize{blocksize} {
        mapBlockHandle = pdb::makeObject<MapBlock>(blocksize * 64, blocksize * 64);
    }

    int size() const {
        return blockSize;
    }
};
} // namespace pdb

#endif