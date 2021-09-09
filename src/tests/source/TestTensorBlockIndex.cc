
#ifndef TEST_TENSOR_BLOCK_INDEX_CC
#define TEST_TENSOR_BLOCK_INDEX_CC

#include "PDBString.h"
#include "PDBMap.h"
#include "TensorBlockIndex.h"
#include "InterfaceFunctions.h"

#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

using namespace pdb;

int main() {

    makeObjectAllocatorBlock(124 * 1024 * 1024, true);
    TensorBlockIndex indexes;


    //creating six blocks
    std::vector<int> dimensions1;
    dimensions1.push_back(0);
    dimensions1.push_back(1);
    TensorBlockMeta deduplicatedBlock1(dimensions1);

    std::vector<int> dimensions2;
    dimensions2.push_back(1);
    dimensions2.push_back(2);
    TensorBlockMeta deduplicatedBlock2(dimensions2);

    std::vector<int> dimensions3;
    dimensions3.push_back(3);
    dimensions3.push_back(2);
    TensorBlockMeta deduplicatedBlock3(dimensions3);

    std::vector<int> dimensions4;
    dimensions4.push_back(100);
    dimensions4.push_back(20);
    Handle<TensorBlockMeta>  block4 = makeObject<TensorBlockMeta>(dimensions4);

    std::vector<int> dimensions5;
    dimensions5.push_back(17);
    dimensions5.push_back(18);
    Handle<TensorBlockMeta>  block5 = makeObject<TensorBlockMeta>(dimensions5);     

    std::vector<int> dimensions6;
    dimensions6.push_back(32);
    dimensions6.push_back(1);
    Handle<TensorBlockMeta>  block6 = makeObject<TensorBlockMeta>(dimensions6);

    //test insertions
    int key1 = indexes.getSetKey(1, 8021, 1);
    indexes.insertIndex(key1, deduplicatedBlock1, block4);
    indexes.insertIndex(key1, deduplicatedBlock2, block5);
    int key2 = indexes.getSetKey(1, 8021, 2);
    indexes.insertIndex(key2, deduplicatedBlock3, block6);

    //test queries
    Handle<TensorBlockMeta> targetBlock1 = indexes.getTargetMetadata(1, 8021, 1, deduplicatedBlock1);
    targetBlock1->print();
    Handle<TensorBlockMeta> targetBlock2 = indexes.getTargetMetadata(1, 8021, 1, deduplicatedBlock2);
    targetBlock2->print();
    Handle<TensorBlockMeta> targetBlock3 = indexes.getTargetMetadata(1, 8021, 2, deduplicatedBlock3);
    targetBlock3->print();

    // test serialization
    indexes.serializeIndex("testfile");
    
    // test deserialization
    TensorBlockIndex newIndexes("testfile");
    (indexes.getTargetMetadata(1, 8021, 1, deduplicatedBlock1))->print();
    (indexes.getTargetMetadata(1, 8021, 1, deduplicatedBlock2))->print();
    (indexes.getTargetMetadata(1, 8021, 2, deduplicatedBlock3))->print();
}

#endif
