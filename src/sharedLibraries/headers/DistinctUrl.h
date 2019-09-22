#pragma once

#include <Object.h>
#include <PDBString.h>

namespace pdb {

class DistinctUrl : public pdb::Object {
 public:

  DistinctUrl() = default;

  explicit DistinctUrl(const std::string &url) : url(url) {}

  ENABLE_DEEP_COPY

  // the link is from this page
  pdb::String url;

  // return the url
  pdb::String& getKey() {
    return url;
  }

  // we are returning a value to this object, since we are only interested in doing a distinct query
  // we are simply going to increment this by 0 essentially a NOOP, if incremented by anything else there is going to be a problem
  int& getValue() {
    return *(int*)this;
  }
};

}