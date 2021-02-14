#ifndef COMMENT_FEATURES_TO_CHUNKS_H
#define COMMENT_FEATURES_TO_CHUNKS_H

#include "MultiSelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "CommentFeatureChunks.h"
#include "CommentFeatures.h"

#include <cassert>

using namespace pdb;

namespace reddit {

// template <class OutputClass, class InputClass, class KeyClass, class
// ValueClass>
class CommentFeaturesToChunks
    : public MultiSelectionComp<CommentFeatures, CommentFeatures> {

public:
  ENABLE_DEEP_COPY

  int block_y;

  CommentFeaturesToChunks() : block_y(100) {}

  CommentFeaturesToChunks(int block_y) : block_y(block_y) {}

  Lambda<bool> getSelection(Handle<CommentFeatures> checkMe) override {
    return makeLambda(
        checkMe, [](Handle<CommentFeatures> &checkMe) { return true; });
  }

  Lambda<Vector<Handle<CommentFeatures>>>
  getProjection(Handle<CommentFeatures> checkMe) override {
    return makeLambda(checkMe, [this](Handle<CommentFeatures> &checkMe) {
      Vector<Handle<CommentFeatures>> result;
      int num_y_blocks = ceil(checkMe->getFeatureCount() / (double)block_y);

      for (int i = 0; i < num_y_blocks; i++) {
        Handle<CommentFeatures> features = makeObject<CommentFeatures>(checkMe->index);
        features->y_index = i;
        Vector<double> &feature_vec = features->getFeature();
        Vector<double> &src_feature_vec = checkMe->getFeature();

        for (int j = i * block_y; j < i * block_y + block_y; j++) {
          if (j >= src_feature_vec.size()) {
            feature_vec.push_back(0);
          } else {
            feature_vec.push_back(src_feature_vec[j]);
          }
        }

        result.push_back(features);
      }

      assert(result.size() == num_y_blocks);

      return result;
    });
  }
};

} // namespace reddit

#endif
