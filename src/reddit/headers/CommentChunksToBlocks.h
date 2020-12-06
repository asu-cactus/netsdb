#ifndef COMMENT_CHUNKS_TO_BLOCKS_H
#define COMMENT_CHUNKS_TO_BLOCKS_H

#include <cassert>
#include <cmath>

#include "MultiSelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "CommentFeatureChunks.h"
#include "FFMatrixBlock.h"

using namespace pdb;

namespace reddit {

class CommentChunksToBlocks
    : public MultiSelectionComp<::FFMatrixBlock, CommentFeatureChunks> {

public:
  int block_x;
  int block_y;
  bool padding;
  int batch_size;

  ENABLE_DEEP_COPY

  CommentChunksToBlocks() {}

  CommentChunksToBlocks(int block_x, int block_y, bool padding, int batch_size)
      : block_x(block_x), block_y(block_y), padding(padding),
        batch_size(batch_size) {}

  Lambda<bool> getSelection(Handle<CommentFeatureChunks> checkMe) override {
    return makeLambda(
        checkMe, [](Handle<CommentFeatureChunks> &checkMe) { return true; });
  }

  Lambda<Vector<Handle<::FFMatrixBlock>>>
  getProjection(Handle<CommentFeatureChunks> checkMe) override {
    return makeLambda(checkMe, [this](Handle<CommentFeatureChunks> &checkMe) {
      Vector<Handle<::FFMatrixBlock>> result;
      int num_y_blocks = ceil(checkMe->feature_count / (double)block_y);

      // We will only split chunks horizontally here, not vertically
      assert(checkMe->getActualChunkSize() <= block_x);

      int real_block_x = padding ? block_x : checkMe->getActualChunkSize();
      for (int i = 0; i < num_y_blocks; i++) {
        int real_block_y =
            padding ? block_y
                    : min(block_y, checkMe->feature_count - i * block_y);
        Handle<::FFMatrixBlock> myData = makeObject<::FFMatrixBlock>(
            checkMe->chunk_index, i, real_block_x, real_block_y, batch_size,
            checkMe->feature_count);

        Map<int, Vector<double>> &chunk = checkMe->getChunk();

        // Because each chunk has a mapping between the actual index of the
        // comment and the features. These may be stored out of order in the
        // map.
        int offset = checkMe->chunk_index * checkMe->chunk_size;

        for (int x = 0; x < real_block_x; x++) {
          for (int y = 0; y < real_block_y; y++) {
            int curX = x;
            int curY = (i * real_block_y + y);

            double data = curX >= checkMe->getActualChunkSize() ||
                                  curY >= checkMe->feature_count
                              ? 0
                              : chunk[offset + curX][curY];
            (*(myData->getRawDataHandle()))[x * real_block_y + y] = data;
          }
        }

        result.push_back(myData);
      }

      return result;
    });
  }
};

} // namespace reddit

#endif
