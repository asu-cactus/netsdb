#ifndef CONV2D_KERNEL_TO_CHUNKS_H
#define CONV2D_KERNEL_TO_CHUNKS_H

#include "MultiSelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "FFMatrixBlock.h"
#include "ImageChunk.h"
#include "Kernel.h"

#include <cassert>

using namespace pdb;

namespace conv2d_memory_fusion {

// template <class OutputClass, class InputClass, class KeyClass, class
// ValueClass>
class KernelToChunks : public MultiSelectionComp<ImageChunk, Kernel> {

public:
  ENABLE_DEEP_COPY

  int block_y;

  KernelToChunks() {}
  KernelToChunks(int block_y) : block_y(block_y) {}

  Lambda<bool> getSelection(Handle<Kernel> checkMe) override {
    return makeLambda(checkMe, [](Handle<Kernel> &checkMe) { return true; });
  }

  Lambda<Vector<Handle<ImageChunk>>>
  getProjection(Handle<Kernel> checkMe) override {
    return makeLambda(checkMe, [this](Handle<Kernel> &checkMe) {
      Vector<Handle<ImageChunk>> result;

      int y_index = 0;

      int row_width = checkMe->x * checkMe->y * checkMe->z;
      // Kernel flattens to a single row
      // The position of this row is the index of the kernel
      int block_row = 0;
      int block_row_start = checkMe->getKey();

      Handle<ImageChunk> chunk =
          makeObject<ImageChunk>(block_y, block_y, block_row, y_index, block_row_start);

      for (int c = 0; c < checkMe->get_num_channels(); c++) {
        double *data =
            checkMe->getMatrixAtIndex(c)->getValue().rawData->c_ptr();

        for (int i = 0; i < checkMe->x * checkMe->y; i++) {
          if (chunk->getSize() > 0 && chunk->getSize() % block_y == 0) {
            result.push_back(chunk);
            y_index++;
            chunk = makeObject<ImageChunk>(block_y, block_y, block_row, y_index,
                                           block_row_start);
          }

          chunk->getChunk().push_back(data[i]);
        }
      }

      // Add the last chunk of last row
      // Set placeholder for bias
      if (chunk->getSize() % block_y == 0) {
        result.push_back(chunk);
        y_index++;
        chunk =
            makeObject<ImageChunk>(block_y, block_y, block_row_start, y_index, block_row_start);
      }

      chunk->getChunk().push_back(0);
      result.push_back(chunk);

      return result;
    });
  }
};

} // namespace conv2d_memory_fusion

#endif
