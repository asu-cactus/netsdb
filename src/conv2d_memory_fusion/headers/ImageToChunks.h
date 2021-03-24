#ifndef CONV2D_IMAGE_TO_CHUNKS_H
#define CONV2D_IMAGE_TO_CHUNKS_H

#include "MultiSelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "Image.h"
#include "ImageChunk.h"

#include <cassert>

using namespace pdb;

namespace conv2d_memory_fusion {

// template <class OutputClass, class InputClass, class KeyClass, class
// ValueClass>
class ImageToChunks : public MultiSelectionComp<ImageChunk, Image> {

public:
  ENABLE_DEEP_COPY

  int block_y;
  int kernel_size;
  int stride;
  int padding;

  ImageToChunks() {}

  ImageToChunks(int block_y, int kernel_size, int stride, int padding = 0)
      : block_y(block_y), kernel_size(kernel_size), stride(stride), padding(padding) {}

  Lambda<bool> getSelection(Handle<Image> checkMe) override {
    return makeLambda(checkMe, [](Handle<Image> &checkMe) { return true; });
  }

  Lambda<Vector<Handle<ImageChunk>>>
  getProjection(Handle<Image> checkMe) override {
    return makeLambda(checkMe, [this](Handle<Image> &checkMe) {
      Vector<Handle<ImageChunk>> result;
      int num_y_blocks =
          ceil(checkMe->get_conv2d_matrix_cols(kernel_size, stride, padding) /
               (double)block_y);
      int windows = checkMe->get_conv2d_window_count(kernel_size, stride, padding);
      std::cout << "[ImageToChunks] windows: " << windows << ", kernel: " << kernel_size << ", strides: " << stride << ", padding: " << padding << std::endl;
      int channels = checkMe->get_num_channels();
      int row_start = checkMe->getKey() * windows;

      int counter = 0;
      Handle<ImageChunk> chunk = nullptr;

      for (int w = 0; w < windows; w++) {
        if (chunk != nullptr) {
          // Adding the final column of 1's to the row

          if (chunk->getSize() % block_y == 0) {
            result.push_back(chunk);
            counter++;
            chunk = makeObject<ImageChunk>(block_y, block_y, w, counter, row_start);
          }

          chunk->getChunk().push_back(1);
          result.push_back(chunk);
        }

        counter = 0;
        chunk = makeObject<ImageChunk>(block_y, block_y, w, counter, row_start);

        for (int c = 0; c < channels; c++) {
          for (int i = 0; i < kernel_size; i++) {
            for (int j = 0; j < kernel_size; j++) {
              if (chunk->getSize() > 0 && chunk->getSize() % block_y == 0) {
                result.push_back(chunk);
                counter++;
                chunk = makeObject<ImageChunk>(block_y, block_y, w, counter, row_start);
              }

              chunk->getChunk().push_back(
                  checkMe->get_win_value(w, i, j, stride, c, kernel_size, padding));
            }
          }
        }
      }

      // Add the last chunk of last row
      if (chunk->getSize() % block_y == 0) {
        result.push_back(chunk);
        counter++;
        chunk =
            makeObject<ImageChunk>(block_y, block_y, windows - 1, counter, row_start);
      }

      chunk->getChunk().push_back(1);
      result.push_back(chunk);

      return result;
    });
  }
};

} // namespace conv2d_memory_fusion

#endif
