#ifndef CONV2D_IMAGE_BLOCK_TO_MATRIX_H
#define CONV2D_IMAGE_BLOCK_TO_MATRIX_H

#include <cassert>
#include <cmath>

#include "SelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "FFMatrixBlock.h"
#include "ImageBlock.h"

using namespace pdb;

namespace conv2d_memory_fusion {

class ImageBlockToMatrix : public SelectionComp<::FFMatrixBlock, ImageBlock> {

public:
  int block_x;
  int block_y;
  bool padding;
  int batch_size;
  int total_features;

  ENABLE_DEEP_COPY

  ImageBlockToMatrix() {}

  ImageBlockToMatrix(int block_x, int block_y, bool padding, int batch_size,
                     int total_features)
      : block_x(block_x), block_y(block_y), padding(padding),
        batch_size(batch_size), total_features(total_features) {}

  Lambda<bool> getSelection(Handle<ImageBlock> checkMe) override {
    return makeLambda(checkMe,
                      [](Handle<ImageBlock> &checkMe) { return true; });
  }

  Lambda<Handle<::FFMatrixBlock>>
  getProjection(Handle<ImageBlock> checkMe) override {
    return makeLambda(checkMe, [this](Handle<ImageBlock> &checkMe) {
      int real_block_x = padding ? block_x : checkMe->getActualBlockSize();
      int real_block_y =
          padding
              ? block_y
              : min(block_y, total_features - checkMe->block_y_index * block_y);

      Handle<::FFMatrixBlock> myData = makeObject<::FFMatrixBlock>(
          checkMe->block_x_index, checkMe->block_y_index, real_block_x,
          real_block_y, batch_size, total_features, true);

      Map<int, Vector<double>> &chunk = checkMe->getBlock();
      // Because each chunk has a mapping between the actual index of the
      // comment and the features. These may be stored out of order in the
      // map.
      int offset = checkMe->block_x_index * checkMe->block_x_size;

      for (int x = 0; x < real_block_x; x++) {
        for (int y = 0; y < real_block_y; y++) {
          double data =
              x >= checkMe->getActualBlockSize() || y >= checkMe->feature_count
                  ? 0
                  : chunk[offset + x][y];
          (*(myData->getRawDataHandle()))[x * real_block_y + y] = data;
        }
      }

      return myData;
    });
  }
};

} // namespace conv2d_memory_fusion

#endif
