#ifndef REDDIT_COMMENT_INFERENCE_JOIN_H
#define REDDIT_COMMENT_INFERENCE_JOIN_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include <JoinComp.h>

#include "RedditComment.h"
#include "InferenceResult.h"

#include <iostream>

using namespace pdb;

namespace reddit {

class CommentInferenceJoin
    : public JoinComp<Comment, ::InferenceResult, Comment> {

public:
  ENABLE_DEEP_COPY

  CommentInferenceJoin() = default;

  Lambda<bool> getSelection(Handle<::InferenceResult> in1,
                            Handle<Comment> in2) override {
    return makeLambdaFromMethod(in1, getKey) ==
           makeLambdaFromMethod(in2, get_index);
  }

  Lambda<Handle<Comment>>
  getProjection(Handle<::InferenceResult> in1,
                Handle<Comment> in2) override {
    return makeLambda(
        in1, in2, [](Handle<::InferenceResult> &in1, Handle<Comment> &in2) {
          Handle<Comment> tmp = makeObject<Comment>(*in2);
          tmp->label = in1->getLabel();
          return tmp;
        });
  }
};

} // namespace reddit

#endif
