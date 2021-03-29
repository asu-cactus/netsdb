#ifndef CONV2D_IMAGE_CHUNKS_TO_BLOCK_H
#define CONV2D_IMAGE_CHUNKS_TO_BLOCK_H

#include <cassert>
#include <cmath>

#include "ClusterAggregateComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "ImageBlock.h"
#include "ImageChunk.h"

using namespace pdb;

namespace conv2d_memory_fusion {

class ImageChunksToBlock
    : public ClusterAggregateComp<ImageBlock, ImageChunk, long, ImageBlock> {

public:
  ENABLE_DEEP_COPY

  int chunk_size;

  ImageChunksToBlock() {}

  ImageChunksToBlock(int chunk_size) : chunk_size(chunk_size) {}

  // the key type must have == and size_t hash () defined
  Lambda<long> getKeyProjection(Handle<ImageChunk> aggMe) override {
    return makeLambda(aggMe, [this](Handle<ImageChunk> &aggMe) {
      long a = (long)(aggMe->getChunkActualRowIndex() / chunk_size);
      long b = aggMe->y_index;
      long c = a * 10000 + b;
      return c;
    });
  }

  // the value type must have + defined
  Lambda<ImageBlock> getValueProjection(Handle<ImageChunk> aggMe) override {
    return makeLambda(aggMe, [this](Handle<ImageChunk> &aggMe) {
      Handle<ImageBlock> temp = makeObject<ImageBlock>(*aggMe, chunk_size);
      return *temp;
    });
  }
};

} // namespace conv2d_memory_fusion

#endif
