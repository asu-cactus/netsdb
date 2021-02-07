#pragma once

#include <Object.h>
#include <Handle.h>
#include <PDBString.h>
#include <PDBVector.h>
namespace pdb {

class Link : public pdb::Object {
public:

  Link() = default;

  Link(const int url) : url(url) {
  
       neighbors = makeObject<Vector<int>>();

  }

  void addNeighbor(const int neighbor) {
  
       neighbors->push_back(neighbor);

  }

  ENABLE_DEEP_COPY

  // the url
  int url;

  // the neighbors of the url
  Handle<Vector<int>> neighbors;

  size_t hash() const override{
        return url;
  }

};

}
