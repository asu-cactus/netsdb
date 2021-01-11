#ifndef COMMENT_FEATURES_TO_CHUNKS_H
#define COMMENT_FEATURES_TO_CHUNKS_H

#include "ClusterAggregateComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "CommentFeatureChunks.h"
#include "CommentFeatures.h"

using namespace pdb;

namespace reddit {

// template <class OutputClass, class InputClass, class KeyClass, class
// ValueClass>
class CommentFeaturesToChunks
    : public ClusterAggregateComp<CommentFeatureChunks, CommentFeatures, int,
                                  CommentFeatureChunks> {

public:
  ENABLE_DEEP_COPY

  int chunk_size;

  CommentFeaturesToChunks() : chunk_size(100) {}

  CommentFeaturesToChunks(int chunk_size) : chunk_size(chunk_size) {}

  // the key type must have == and size_t hash () defined
  Lambda<int> getKeyProjection(Handle<CommentFeatures> aggMe) override {
    return makeLambda(aggMe, [this](Handle<CommentFeatures> &aggMe) {
      return (int)(aggMe->index / chunk_size);
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
