#pragma once

#include <JoinComp.h>
#include <LambdaCreationFunctions.h>
#include "RedditComment.h"
#include "RedditSub.h"
#include "RedditSubsAndComments.h"

namespace reddit {

class JoinSubsWithComments : public JoinComp<SubsAndComments, Comment, Sub> {

 public:

  ENABLE_DEEP_COPY

  JoinSubsWithComments() = default;

  Lambda<bool> getSelection(Handle<Comment> in1, Handle<Sub> in2) override {
    return (makeLambdaFromMember(in1, subreddit) == makeLambdaFromMember(in2, name));
  }

  Lambda<Handle<SubsAndComments>> getProjection(Handle<Comment> in1, Handle<Sub> in2) override {
    return makeLambda(in1, in2, [](Handle<Comment>& in1, Handle<Sub>& in2) {
      Handle<SubsAndComments> tmp = makeObject<SubsAndComments>(in1, in2);
      return tmp;
    });
  }
};

}
