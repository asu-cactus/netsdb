#pragma once

#include <Object.h>
#include <LambdaCreationFunctions.h>
#include "LinkWithValue.h"
#include "MultiSelectionComp.h"
#include "URLURLsRank.h"

namespace pdb {

class URLRankMultiSelection : public MultiSelectionComp<LinkWithValue, URLURLsRank> {

public:
  
  ENABLE_DEEP_COPY

  URLRankMultiSelection() = default;

  Lambda<bool> getSelection(Handle<URLURLsRank> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<URLURLsRank>& checkMe) { return true; });
  }

  Lambda<Vector<Handle<LinkWithValue>>> getProjection(Handle<URLURLsRank> checkMe) override {
        return makeLambda(checkMe, [](Handle<URLURLsRank>& checkMe) {
            Vector<Handle<LinkWithValue>> myVec;
	    int numUrls = checkMe->urls->size();
	    Vector<int>& urls = *(checkMe->urls);
            for (int i = 0; i < numUrls; i++) {
                Handle<LinkWithValue> urlWithRank =
                    makeObject<LinkWithValue>(urls[i], checkMe->rank/numUrls);
                myVec.push_back(urlWithRank);
            }
            return myVec;
        });
    }


};

}
