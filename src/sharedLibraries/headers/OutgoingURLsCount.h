#pragma once

#include "Object.h"
#include "PDBString.h"
#include "Link.h"

namespace pdb {

class OutgoingURLsCount : public pdb::Object {
 public:

  OutgoingURLsCount() = default;

  explicit OutgoingURLsCount(const std::string &from) : from(from) {}

  ENABLE_DEEP_COPY

  // the url from which we are coming
  String from;

  // the number of outgoing links
  uint64_t outgoingCount = 0;

  // return the url
  String& getKey() {
    return from;
  }

  // return the value
  uint64_t& getValue() {
    return outgoingCount;
  }

};

}