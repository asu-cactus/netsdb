#pragma once

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "Link.h"
#include "RankedUrl.h"
#include "SelectionComp.h"

namespace pdb {

class DistinctProjection : public SelectionComp<RankedUrl, Link> {
public:

  ENABLE_DEEP_COPY

  DistinctProjection() = default;

  Lambda<bool> getSelection(Handle<Link> checkMe) override {
    return makeLambda(checkMe, [](Handle<Link>& checkMe) {
      return true;
    });
  }

  Lambda<Handle<RankedUrl>> getProjection(Handle<Link> checkMe) override {
    return makeLambda(checkMe, [](Handle<Link>& checkMe) {
      Handle<RankedUrl> tmp = makeObject<RankedUrl>(checkMe->url, 0.25f);
      return tmp;
    });
  }
};


}
