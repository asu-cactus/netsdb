#pragma once

#include <Object.h>
#include <Handle.h>
#include <PDBVector.h>

namespace pdb {

class URLURLsRank : public pdb::Object {
 public:

  URLURLsRank() = default;

  URLURLsRank(int url, Handle<Vector<int>> urls, float rank) : url(url), urls(urls), rank(rank) {}

  ENABLE_DEEP_COPY

  // this url
  int url;

  // neighbors of this url
  Handle<Vector<int>> urls;

  // the value
  float rank = 0.0f;
};

}
