#pragma once

#include <Object.h>
#include <PDBString.h>

namespace pdb {

class Link : public pdb::Object {
public:

  Link() = default;

  Link(const std::string &from, const std::string &to) : from(from), to(to) {}

  ENABLE_DEEP_COPY

  // the link is from this page
  pdb::String from;

  // to this page
  pdb::String to;
};

}