
#ifndef USE_TEMP_ALLOCATION_BLOCK_H
#define USE_TEMP_ALLOCATION_BLOCK_H

#include "Allocator.h"
#include "InterfaceFunctions.h"
#include <memory>

namespace pdb {

class UseTemporaryAllocationBlock;
typedef std::shared_ptr<UseTemporaryAllocationBlock> UseTemporaryAllocationBlockPtr;

class UseTemporaryAllocationBlock {

    AllocatorState oldInfo;

public:
    explicit UseTemporaryAllocationBlock(void* memory, size_t size) {
        oldInfo = getAllocator().temporarilyUseBlockForAllocations(memory, size);
    }

    explicit UseTemporaryAllocationBlock(size_t size) {
        oldInfo = getAllocator().temporarilyUseBlockForAllocations(size);
    }

    ~UseTemporaryAllocationBlock() {
        getAllocator().restoreAllocationBlock(oldInfo);
    }

    // forbidden, to avoid double frees
    UseTemporaryAllocationBlock(const UseTemporaryAllocationBlock&) = delete;
    UseTemporaryAllocationBlock& operator=(const UseTemporaryAllocationBlock&) = delete;
};
}

#endif
