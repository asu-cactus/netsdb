
#ifndef COMMENT_FEATURE_CHUNKS_H
#define COMMENT_FEATURE_CHUNKS_H


#include <cassert>

#include "Handle.h"
#include "Object.h"
#include "PDBMap.h"
#include "PDBString.h"
#include "PDBVector.h"

#include "CommentFeatures.h"
#include "FFMatrixBlock.h"
#include "FFMatrixData.h"
#include "FFMatrixMeta.h"

using namespace pdb;

namespace reddit {

class CommentFeatureChunks : public pdb::Object {

public:
  int chunk_index;
  int chunk_size;
  int feature_count;
  Handle<Map<int, Vector<double>>> feature_chunk = nullptr;

  ENABLE_DEEP_COPY

  // Default constructor:
  CommentFeatureChunks() {}

  // Default destructor:
  ~CommentFeatureChunks() {}

  // Constructor with arguments:
  CommentFeatureChunks(CommentFeatures &feature, int chunk_size)
      : chunk_size(chunk_size), chunk_index((int)(feature.index / chunk_size)) {
    feature_chunk = makeObject<Map<int, Vector<double>>>();
    (*feature_chunk)[feature.index] = *(feature.features);

    feature_count = feature.features->size();
  }

  Map<int, Vector<double>> &getChunk() { return *feature_chunk; }

  int &getKey() { return this->chunk_index; }

  int getActualChunkSize() { return feature_chunk->size(); }

  CommentFeatureChunks &getValue() { return *this; }

  void dump() {
    Map<int, Vector<double>> &rhs = getChunk();
    auto iter = rhs.begin();
    while (iter != rhs.end()) {
      int myKey = (*iter).key;
      Vector<double> &data = (*iter).value;
      for (int i = 0; i < data.size(); i++)
        std::cout << data[i] << ", ";
      std::cout << std::endl;
    }
  }

  CommentFeatureChunks &operator+(CommentFeatureChunks &addMeIn) {

    Map<int, Vector<double>> &rhs = addMeIn.getChunk();
    auto iter = rhs.begin();
    while (iter != rhs.end()) {
      int myKey = (*iter).key;
      if (feature_chunk->count(myKey) != 0) {
        std::cout << "[COMMENT FEATURE CHUNKS] Failed Aggregation! Expected count of " << myKey << " to be 0, but was " << feature_chunk->count(myKey) << std::endl;
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
