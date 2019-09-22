#pragma once

#include <JoinComp.h>
#include <LambdaCreationFunctions.h>
#include "LinkWithValue.h"
#include "Link.h"
#include "RankedUrl.h"

namespace pdb {

class JoinRankedUrlWithLink : public JoinComp<LinkWithValue, Link, RankedUrl> {

 public:

  ENABLE_DEEP_COPY

  JoinRankedUrlWithLink() = default;

  Lambda<bool> getSelection(Handle<Link> in1, Handle<RankedUrl> in2) override {
    return (makeLambdaFromMember(in1, from) == makeLambdaFromMember(in2, url));
  }

  Lambda<Handle<LinkWithValue>> getProjection(Handle<Link> in1, Handle<RankedUrl> in2) override {
    return makeLambda(in1, in2, [](Handle<Link>& in1, Handle<RankedUrl>& in2) {
      Handle<LinkWithValue> tmp = makeObject<LinkWithValue>(in1->from, in1->to, in2->rank);
      return tmp;
    });
  }
};

}
