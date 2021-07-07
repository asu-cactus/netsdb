
#ifndef CONV2D_IMAGE_BLOCK_H
#define CONV2D_IMAGE_BLOCK_H

#include <cassert>

#include "Handle.h"
#include "Object.h"
#include "PDBMap.h"
#include "PDBString.h"
#include "PDBVector.h"

#include "ImageChunk.h"

using namespace pdb;

namespace conv2d_memory_fusion {

class ImageBlock : public pdb::Object {

public:
  int block_x_index;
  int block_x_size;
  int block_y_size;
  int block_y_index;
  Map<int, Vector<double>> chunks;
  long key;

  int feature_count;

  ENABLE_DEEP_COPY

  // Default constructor:
  ImageBlock() {}

  // Default destructor:
  ~ImageBlock() {}

  // Constructor with arguments:
  ImageBlock(ImageChunk &chunk, int block_x_size)
      : block_y_size(chunk.y_size), block_y_index(chunk.y_index),
        block_x_size(block_x_size),
        block_x_index((int)(chunk.getChunkActualRowIndex() / block_x_size)) {
    feature_count = chunk.getSize();
    chunks[chunk.getChunkActualRowIndex()] = chunk.getChunk();
    key = block_x_index * 10000 + block_y_index;
  }

  Map<int, Vector<double>> &getBlock() { return chunks; }

  long &getKey() { return key; }

  ImageBlock &getValue() { return *this; }

  int getActualBlockSize() { return chunks.size(); }

  void dump() {
    Map<int, Vector<double>> &rhs = getBlock();
    std::cout << "ImageBlock: " << block_x_index << ", " << block_y_index << std::endl;
    std::cout << "block size: " << block_x_size << ", " << block_y_size << ", key: " << key << std::endl; 
    auto iter = rhs.begin();
    while (iter != rhs.end()) {
      int myKey = (*iter).key;
      std::cout << "Key: " << myKey << " -> ";
      Vector<double> &data = (*iter).value;
      for (int i = 0; i < data.size(); i++)
        std::cout << data[i] << ", ";
      std::cout << std::endl;
      ++iter;
    }
  }

  ImageBlock &operator+(ImageBlock &addMeIn) {

    Map<int, Vector<double>> &rhs = addMeIn.getBlock();
    auto iter = rhs.begin();
    while (iter != rhs.end()) {
      int myKey = (*iter).key;
      if (chunks.count(myKey) == 0) {
        chunks[myKey] = (*iter).value;
        //std::cout << "[ImageBlock] Merging bucket " << key << " adding key "
                  //<< myKey << std::endl;
      } else {
        //std::cout << "[ImageBlock] Merging bucket " << key << " NOT adding key "
                  //<< myKey << std::endl;
      }
      ++iter;
    }
    return *this;
  }
};

} // namespace conv2d_memory_fusion

#endif
