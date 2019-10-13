#pragma once

#include <Object.h>
#include <PDBString.h>

namespace pdb {

class LinkWithValue : public pdb::Object {
 public:

  LinkWithValue() = default;

  LinkWithValue(int url, float rank) : url(url), rank(rank) {}

  ENABLE_DEEP_COPY

  // this url
  int url;


  // the value
  float rank = 0.0f;
};

}
