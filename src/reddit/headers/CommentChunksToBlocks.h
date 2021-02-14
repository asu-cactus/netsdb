#ifndef COMMENT_CHUNKS_TO_BLOCKS_H
#define COMMENT_CHUNKS_TO_BLOCKS_H

#include <cassert>
#include <cmath>

#include "ClusterAggregateComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "CommentFeatureChunks.h"
#include "FFMatrixBlock.h"

using namespace pdb;

namespace reddit {

class CommentChunksToBlocks
    : public ClusterAggregateComp<CommentFeatureChunks, CommentFeatures, long,
                                  CommentFeatureChunks>  {

public:
ENABLE_DEEP_COPY

  int chunk_size;

  CommentChunksToBlocks() : chunk_size(100) {}

  CommentChunksToBlocks(int chunk_size) : chunk_size(chunk_size) {}

  // the key type must have == and size_t hash () defined
  Lambda<long> getKeyProjection(Handle<CommentFeatures> aggMe) override {
    return makeLambda(aggMe, [this](Handle<CommentFeatures> &aggMe) {
      long a = (long)(aggMe->index / chunk_size);
      long b = aggMe->y_index;
      long c = a*10000 + b;
      return c;
    });
  }

  // the value type must have + defined
  Lambda<CommentFeatureChunks>
  getValueProjection(Handle<CommentFeatures> aggMe) override {
    return makeLambda(aggMe, [this](Handle<CommentFeatures> &aggMe) {
      Handle<CommentFeatureChunks> temp =
          makeObject<CommentFeatureChunks>(*aggMe, chunk_size);
      return *temp;
    });
  }

};

} // namespace reddit

#endif
