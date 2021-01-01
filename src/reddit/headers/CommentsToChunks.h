#ifndef COMMENTS_TO_CHUNKS_H
#define COMMENTS_TO_CHUNKS_H

#include "ClusterAggregateComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "CommentsChunk.h"
#include "RedditComment.h"

using namespace pdb;

namespace reddit {

// template <class OutputClass, class InputClass, class KeyClass, class
// ValueClass>
class CommentsToChunks
    : public ClusterAggregateComp<CommentsChunk, Comment, int,
                                  CommentsChunk> {

public:
  ENABLE_DEEP_COPY

  int chunk_size;

  CommentsToChunks() : chunk_size(100) {}

  CommentsToChunks(int chunk_size) : chunk_size(chunk_size) {}

  // the key type must have == and size_t hash () defined
  Lambda<int> getKeyProjection(Handle<Comment> aggMe) override {
    return makeLambda(aggMe, [this](Handle<Comment> &aggMe) {
      return (int)(aggMe->index / chunk_size);
    });
  }

  // the value type must have + defined
  Lambda<CommentsChunk>
  getValueProjection(Handle<Comment> aggMe) override {
    return makeLambda(aggMe, [this](Handle<Comment> &aggMe) {
      Handle<CommentsChunk> temp =
          makeObject<CommentsChunk>(*aggMe, chunk_size);
      return *temp;
    });
  }
};

} // namespace reddit

#endif
