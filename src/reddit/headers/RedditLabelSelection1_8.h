#pragma once

#include <SelectionComp.h>
#include <LambdaCreationFunctions.h>
#include "RedditComment.h"

namespace reddit {

class LabelSelection1_8 : public SelectionComp<Comment, Comment> {

 public:

  ENABLE_DEEP_COPY

  LabelSelection1_8() = default;

  bool positive = true;
  int threshold = 8;

  Lambda<bool> getSelection(Handle<Comment> in) override {
    //return (makeLambdaFromMember(in, label) == makeLambda(in, [](Handle<Comment>& in){return 1;}));
    return makeLambda(in, [&](Handle<Comment>& in) { 
        if (positive == false)
            return in->label < threshold;
        else
            return in->label >= threshold; });
  }

  Lambda<Handle<Comment>> getProjection(Handle<Comment> in) override {
    return makeLambda(in, [](Handle<Comment>& in) {
      Handle<Comment> tmp = makeObject<Comment>(*in);
      return tmp;
    });
  }
};

}
