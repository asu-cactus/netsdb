#ifndef CONV2D_CONV_RESULT_TO_CHUNKS_H
#define CONV2D_CONV_RESULT_TO_CHUNKS_H

#include "MultiSelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "FFMatrixBlock.h"
#include "ImageChunk.h"

#include <cassert>

using namespace pdb;

namespace conv2d_memory_fusion {

// template <class OutputClass, class InputClass, class KeyClass, class
// ValueClass>
class ConvResultToChunks
    : public MultiSelectionComp<ImageChunk, ::FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  int num_images;

  ConvResultToChunks() {}

  ConvResultToChunks(int num_images) : num_images(num_images) {}

  Lambda<bool> getSelection(Handle<::FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe,
                      [](Handle<::FFMatrixBlock> &checkMe) { return true; });
  }

  Lambda<Vector<Handle<ImageChunk>>>
  getProjection(Handle<::FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe, [this](Handle<::FFMatrixBlock> &checkMe) {
      Vector<Handle<ImageChunk>> result;

      assert(checkMe->getTotalRowNums() % num_images == 0);
      int group_size = checkMe->getTotalRowNums() / num_images;
      std::cout << "[ConvResultToChunks] BLOCK ID: "
                << checkMe->getBlockRowIndex() << ", "
                << checkMe->getBlockColIndex() << std::endl;

      int origin_x = checkMe->getBlockRowIndex() * checkMe->getRowNums();
      int origin_y = checkMe->getBlockColIndex() * checkMe->getColNums();

      std::cout << "[ConvResultToChunks] ORIGIN: " << origin_x << ", "
                << origin_y << std::endl;
      std::cout << "[ConvResultToChunks] GROUP_SIZE: " << group_size
                << std::endl;

      // Strip away the padding
      int real_x = checkMe->isLastRowBlock()
                       ? checkMe->getTotalRowNums() % checkMe->getRowNums()
                       : checkMe->getRowNums();
      int real_y = checkMe->isLastColBlock()
                       ? checkMe->getTotalColNums() % checkMe->getColNums()
                       : checkMe->getColNums();

      int image_id = floor(origin_x / group_size);
      int done = origin_x % group_size;
      int left = min(group_size - done, real_x);
      int size = left * real_y;

      std::cout << "[ConvResultToChunks] image_id: " << image_id
                << ", done: " << done << ", left: " << left
                << ", size: " << size << std::endl;

      /*
        size = left * real_y or if left > checkMe->getRowNums(),
        checkMe->getRowNums() * real_y block_row = image_id y_index =
        checkMe->getBlockColIndex() block_row_start = done
      */

      int i = 0;
      Handle<ImageChunk> chunk = nullptr;
      double *data = checkMe->getValue().rawData->c_ptr();

      while (i < real_x) {
        for (int j = 0; j < real_y; j++) {
          chunk = makeObject<ImageChunk>(
              left, 1, done,
              checkMe->getBlockColIndex() * checkMe->getColNums() + j, image_id);
          for (int pos = 0; pos < left; pos++) {
            chunk->getChunk().push_back(
                data[(i + pos) * checkMe->getColNums() + j]);
          }
          result.push_back(chunk);
        }

        i += left;
        left = min(real_x - i, group_size);
        image_id++;
        done = 0;
        size = left * real_y;
      }

      return result;
    });
  }
};

} // namespace conv2d_memory_fusion

#endif
