#pragma once

#include <SelectionComp.h>
#include <LambdaCreationFunctions.h>
#include "RedditComment.h"

namespace reddit {

class RedditLabelProjection : public SelectionComp<Comment, Comment> {

 public:

  ENABLE_DEEP_COPY

  double labelProbability = 0.0;

  RedditLabelProjection() = default;

  RedditLabelProjection(double probability) {
     if (probability < 0) {
        this->labelProbability = 0.0; 
     } else if (probability > 1) {
        this->labelProbability = 1.0;
     } else {
        this->labelProbability = probability;
     }
     srand (time(NULL));
  }

  Lambda<bool> getSelection(Handle<Comment> in) override {
    return makeLambda(in, [](Handle<Comment>& in) { return true;});
  }

  Lambda<Handle<Comment>> getProjection(Handle<Comment> in) override {
    return makeLambda(in, [&](Handle<Comment>& in) {
      int label = 0;
      double sampledProb = double(rand()%10)/double(10);
      if(sampledProb < this->labelProbability) {
          label = 1;
      } else {
          label = -1;
      }
      in->label = label;
      Handle<Comment> tmp = makeObject<Comment>(*in);
      return tmp;
    });
  }
};

}
