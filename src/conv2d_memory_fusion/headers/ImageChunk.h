#ifndef CONV2D_IMAGE_CHUNK_H
#define CONV2D_IMAGE_CHUNK_H

#include "Handle.h"
#include "PDBObject.h"
#include "PDBVector.h"

namespace conv2d_memory_fusion {
class ImageChunk : public pdb::Object {
public:
  int block_row;
  int y_index;
  pdb::Vector<float> chunk;
  int chunk_size;
  int y_size;

  int block_row_start;

  ENABLE_DEEP_COPY
  ImageChunk() {}

  ImageChunk(int size, int y_size, int block_row, int y_index, int block_row_start)
      : chunk_size(size), y_size(y_size), block_row(block_row),
        y_index(y_index), block_row_start(block_row_start) {
    chunk.resize(size);
  }

  int getChunkActualRowIndex() { return block_row_start + block_row; }

  pdb::Vector<float> &getChunk() { return chunk; }

  void dump() {
    std::cout << "size: " << chunk_size << ", block_row: " << block_row << ", y_index: " << y_index << ", block_row_start" << block_row_start << std::endl;
    for (int i = 0; i < chunk.size(); i++) {
      std::cout << chunk[i] << ",";
    }
    std::cout << std::endl;
  }

  int getSize() { return chunk.size(); }
};
} // namespace conv2d_memory_fusion

#endif