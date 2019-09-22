#pragma once

#include <Object.h>
#include <PDBString.h>

namespace pdb {

class RankedUrl : public pdb::Object {
public:

  RankedUrl() = default;

  RankedUrl(const pdb::String &url, float rank) : url(url), rank(rank) {}

  ENABLE_DEEP_COPY

  // the link is from this page
  pdb::String url;

  // the rank
  float rank = 0;

  // return the url
  pdb::String& getKey() {
    return url;
  }

  // return the rank
  float& getValue() {
    return rank;
  }
};

}