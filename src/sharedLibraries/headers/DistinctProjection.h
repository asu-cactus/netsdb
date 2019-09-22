#pragma once

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "DistinctUrl.h"
#include "RankedUrl.h"
#include "SelectionComp.h"

namespace pdb {

class DistinctProjection : public SelectionComp<RankedUrl, DistinctUrl> {
public:

  ENABLE_DEEP_COPY

  DistinctProjection() = default;

  Lambda<bool> getSelection(Handle<DistinctUrl> checkMe) override {
    return makeLambda(checkMe, [](Handle<DistinctUrl>& checkMe) {
      return true;
    });
  }

  Lambda<Handle<RankedUrl>> getProjection(Handle<DistinctUrl> checkMe) override {
    return makeLambda(checkMe, [](Handle<DistinctUrl>& checkMe) {
      Handle<RankedUrl> tmp = makeObject<RankedUrl>(checkMe->getKey(), 0.25f);
      return tmp;
    });
  }
};


}
