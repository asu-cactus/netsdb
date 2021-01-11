#ifndef COMMENT_CHUNK_TO_COMMENTS_H
#define COMMENT_CHUNK_TO_COMMENTS_H

#include <cassert>
#include <cmath>

#include "MultiSelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "CommentsChunk.h"
#include "RedditComment.h"

using namespace pdb;

namespace reddit {

class CommentChunkToComments
    : public MultiSelectionComp<Comment, CommentsChunk> {

public:
  int block_x;
  int block_y;
  bool padding;
  int batch_size;

  ENABLE_DEEP_COPY

  CommentChunkToComments() {}

  Lambda<bool> getSelection(Handle<CommentsChunk> checkMe) override {
    return makeLambda(checkMe,
                      [](Handle<CommentsChunk> &checkMe) { return true; });
  }

  Lambda<Vector<Handle<Comment>>>
  getProjection(Handle<CommentsChunk> checkMe) override {
    return makeLambda(checkMe, [this](Handle<CommentsChunk> &checkMe) {
      Vector<Handle<Comment>> result;

      Map<int, reddit::Comment> &rhs = checkMe->getChunk();
      auto iter = rhs.begin();
      while (iter != rhs.end()) {
        Handle<Comment> myData = makeObject<Comment>((*iter).value);
        result.push_back(myData);
        ++iter;
      }

      return result;
    });
  }
};

} // namespace reddit

#endif
