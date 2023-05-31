#ifndef SPARSEMATRIXBLOCK_H
#define SPARSEMATRIXBLOCK_H

#include "Handle.h"
#include "PDBVector.h"

// PRELOAD %SparseMatrixBlock%

namespace pdb {

class SparseMatrixBlock : public pdb::Object {
    ENABLE_DEEP_COPY
    // Type alias
    using MapBlock = Vector<Handle<Map<int, float>>>;

  public:
    int blockID;
    int blockSize;
    Handle<MapBlock> mapBlockHandle;

    SparseMatrixBlock() = default;
    ~SparseMatrixBlock() = default; // TODO: Do we need to free memory in destructor?
    SparseMatrixBlock(int blockid, int blocksize)
        : blockID{blockid}, blockSize{blocksize} {
        mapBlockHandle = makeObject<MapBlock>(blocksize, blocksize);
        for (int i = 0; i < blocksize; i++) {
            try {
                (*mapBlockHandle)[i] = makeObject<Map<int, float>>(64);
            } catch (pdb::NotEnoughSpace &n) {
                throw;
            }
        }
    }

    int size() const {
        return blockSize;
    }
};
} // namespace pdb

#endif