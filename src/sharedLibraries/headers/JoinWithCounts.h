#pragma once

#include <JoinComp.h>
#include <LambdaCreationFunctions.h>
#include "LinkWithValue.h"
#include "RankedUrl.h"
#include "OutgoingURLsCount.h"

namespace pdb {

class JoinWithCounts : public JoinComp<LinkWithValue, LinkWithValue, OutgoingURLsCount> {
public:

  ENABLE_DEEP_COPY

  JoinWithCounts() = default;

  Lambda<bool> getSelection(Handle<LinkWithValue> in1, Handle<OutgoingURLsCount> in2) override {
    return (makeLambdaFromMember(in1, from) == makeLambdaFromMember(in2, from));
  }

  // basically we just divide by the count
  Lambda<Handle<LinkWithValue>> getProjection(Handle<LinkWithValue> in1, Handle<OutgoingURLsCount> in2) override {
    return makeLambda(in1, in2, [](Handle<LinkWithValue>& in1, Handle<OutgoingURLsCount>& in2) {
      Handle<LinkWithValue> tmp = makeObject<LinkWithValue>(in1->from, in1->to, in1->value / in2->outgoingCount);
      return tmp;
    });
  }
};

}