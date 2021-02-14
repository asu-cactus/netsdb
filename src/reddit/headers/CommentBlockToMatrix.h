#ifndef COMMENT_BLOCK_TO_MATRIX_H
#define COMMENT_BLOCK_TO_MATRIX_H

#include <cassert>
#include <cmath>

#include "SelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "CommentFeatureChunks.h"
#include "FFMatrixBlock.h"

using namespace pdb;

namespace reddit {

class CommentBlockToMatrix
    : public SelectionComp<::FFMatrixBlock, CommentFeatureChunks> {

public:
  int block_x;
  int block_y;
  bool padding;
  int batch_size;
  int total_features;

  ENABLE_DEEP_COPY

  CommentBlockToMatrix() {}

  CommentBlockToMatrix(int block_x, int block_y, bool padding, int batch_size, int total_features)
      : block_x(block_x), block_y(block_y), padding(padding),
        batch_size(batch_size), total_features(total_features) {}

  Lambda<bool> getSelection(Handle<CommentFeatureChunks> checkMe) override {
    return makeLambda(
        checkMe, [](Handle<CommentFeatureChunks> &checkMe) { return true; });
  }

  Lambda<Handle<::FFMatrixBlock>>
  getProjection(Handle<CommentFeatureChunks> checkMe) override {
    return makeLambda(checkMe, [this](Handle<CommentFeatureChunks> &checkMe) {

        int real_block_x = padding ? block_x : checkMe->getActualChunkSize();
        int real_block_y = padding ? block_y : min(block_y, total_features - checkMe->chunk_index * block_y);

        Handle<::FFMatrixBlock> myData = makeObject<::FFMatrixBlock>(
            checkMe->chunk_index, checkMe->chunk_y_index, real_block_x, real_block_y, batch_size,
            total_features, true);

        Map<int, Vector<double>> &chunk = checkMe->getChunk();

        for (int x = 0; x < real_block_x; x++) {
          for (int y = 0; y < real_block_y; y++) {
            double data = x >= checkMe->getActualChunkSize() ||
                                  y >= checkMe->feature_count
                              ? 0
                              : chunk[x][y];
            (*(myData->getRawDataHandle()))[x * real_block_y + y] = data;
          }
        }

        return myData;
    });
  }
};

} // namespace reddit

#endif
