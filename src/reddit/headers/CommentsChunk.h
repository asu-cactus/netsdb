#ifndef SUPPLIER_INFO_H
#define SUPPLIER_INFO_H

#include <cassert>

#include "Handle.h"
#include "Object.h"
#include "PDBMap.h"
#include "PDBString.h"
#include "PDBVector.h"

#include "RedditComment.h"

using namespace pdb;

namespace reddit {

class CommentsChunk : public pdb::Object {

public:
  int chunk_index;
  int chunk_size;
  //   int feature_count;
  Handle<Map<int, Comment>> feature_chunk = nullptr;

  ENABLE_DEEP_COPY

  // Default constructor:
  CommentsChunk() {}

  // Default destructor:
  ~CommentsChunk() {}

  // Constructor with arguments:
  CommentsChunk(Comment &feature, int chunk_size)
      : chunk_size(chunk_size), chunk_index((int)(feature.index / chunk_size)) {
    feature_chunk = makeObject<Map<int, Comment>>();
    (*feature_chunk)[feature.index] = feature;
    // feature_count = feature.features->size();
  }

  int getChunkSize() { return chunk_size; }

  Map<int, Comment> &getChunk() { return *feature_chunk; }

  int &getKey() { return this->chunk_index; }

  int getChunkIndex() { return this->chunk_index; }

  int getActualChunkSize() { return feature_chunk->size(); }

  CommentsChunk &getValue() { return *this; }

  CommentsChunk &operator+(CommentsChunk &addMeIn) {
    Map<int, Comment> &rhs = addMeIn.getChunk();
    auto iter = rhs.begin();
    while (iter != rhs.end()) {
      int myKey = (*iter).key;
      if (feature_chunk->count(myKey) != 0) {
        std::cout << "FAILED!!" << std::endl;
        exit(1);
      }
      (*feature_chunk)[myKey] = (*iter).value;
      ++iter;
    }
    return *this;
  }
};

} // namespace reddit

#endif
