#pragma once

#include <SelectionComp.h>
#include <LambdaCreationFunctions.h>
#include "RedditComment.h"

namespace reddit {

class NegativeLabelSelection : public SelectionComp<Comment, Comment> {

 public:

  ENABLE_DEEP_COPY

  NegativeLabelSelection() = default;

  Lambda<bool> getSelection(Handle<Comment> in) override {
    //return (makeLambdaFromMember(in, label) == makeLambda(in, [](Handle<Comment>& in){return -1;}));
    return makeLambda(in, [](Handle<Comment>& in) {
        if (in != nullptr) {
           return in->label == -1;
        } else {
           std::cout << "Error: null Comment handle" << std::endl;
           return false;
        }
    });
  }

  Lambda<Handle<Comment>> getProjection(Handle<Comment> in) override {
    return makeLambda(in, [](Handle<Comment>& in) {
      if (in != nullptr){
         Handle<Comment> tmp = makeObject<Comment>(*in);
         return tmp;
      } else {
         std::cout << "Error: null Comment handle" << std::endl;
         Handle<Comment> tmp = makeObject<Comment>();
         return tmp;
      }
    });
  }
};

}
