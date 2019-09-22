#pragma once

#include <Object.h>
#include <PDBString.h>

namespace pdb {

class LinkWithValue : public pdb::Object {
 public:

  LinkWithValue() = default;

  LinkWithValue(const std::string &from, const std::string &to, float value) : from(from), to(to), value(value) {}

  ENABLE_DEEP_COPY

  // from this url
  pdb::String from;

  // to this url
  pdb::String to;

  // the value
  float value = 0.0f;
};

}