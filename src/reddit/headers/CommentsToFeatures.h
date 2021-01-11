#ifndef COMMENTS_TO_FEATURES_H
#define COMMENTS_TO_FEATURES_H

#include "CommentFeatures.h"
#include "RedditComment.h"
#include <LambdaCreationFunctions.h>
#include <SelectionComp.h>

namespace reddit {

class CommentsToFeatures : public SelectionComp<CommentFeatures, Comment> {

public:
  ENABLE_DEEP_COPY

  CommentsToFeatures() = default;

  Lambda<bool> getSelection(Handle<Comment> in) override {
    return makeLambda(in, [](Handle<Comment> &in) { return true; });
  }

  Lambda<Handle<CommentFeatures>> getProjection(Handle<Comment> in) override {
    return makeLambda(in, [&](Handle<Comment> &in) {
      Handle<CommentFeatures> tmp = makeObject<CommentFeatures>(in);
      return tmp;
    });
  }
};

} // namespace reddit

#endif