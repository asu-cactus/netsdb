#pragma once

#include <JoinComp.h>
#include <LambdaCreationFunctions.h>
#include "RedditComment.h"
#include "RedditAuthor.h"
#include "RedditSub.h"
#include "RedditFullFeatures.h"

namespace reddit {

class ThreeWayJoin : public JoinComp<FullFeatures, Comment, Author, Sub> {

 public:

  ENABLE_DEEP_COPY

  ThreeWayJoin() = default;

  Lambda<bool> getSelection(Handle<Comment> in1, Handle<Author> in2, Handle<Sub> in3) override {
    return ((makeLambdaFromMember(in1, author) == makeLambdaFromMember(in2, author)) && 
            (makeLambdaFromMember(in1, subreddit) == makeLambdaFromMember(in3, name)));
  }

  Lambda<Handle<FullFeatures>> getProjection(Handle<Comment> in1, Handle<Author> in2, Handle<Sub> in3) override {
    return makeLambda(in1, in2, in3, [](Handle<Comment>& in1, Handle<Author>& in2, Handle<Sub>& in3) {
      Handle<FullFeatures> tmp = makeObject<FullFeatures>(in1, in2, in3);
      return tmp;
    });
  }
};

}
