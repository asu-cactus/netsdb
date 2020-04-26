#pragma once

#include <JoinComp.h>
#include <LambdaCreationFunctions.h>
#include "RedditComment.h"
#include "RedditAuthor.h"
#include "RedditFeatures.h"

namespace reddit {

class JoinAuthorsWithComments : public JoinComp<Features, Comment, Author> {

 public:

  ENABLE_DEEP_COPY

  JoinAuthorsWithComments() = default;

  Lambda<bool> getSelection(Handle<Comment> in1, Handle<Author> in2) override {
    return (makeLambdaFromMember(in1, author) == makeLambdaFromMember(in2, author));
  }

  Lambda<Handle<Features>> getProjection(Handle<Comment> in1, Handle<Author> in2) override {
    return makeLambda(in1, in2, [](Handle<Comment>& in1, Handle<Author>& in2) {
      Handle<Features> tmp = makeObject<Features>(in1, in2);
      return tmp;
    });
  }
};

}
