#ifndef CONV2D_CONV_CHUNKS_TO_IMAGE_H
#define CONV2D_CONV_CHUNKS_TO_IMAGE_H

#include <cassert>
#include <cmath>

#include "ClusterAggregateComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "ImageBlock.h"
#include "Image.h"
#include "FFMatrixBlock.h"

using namespace pdb;

namespace conv2d_memory_fusion {

class ConvChunksToImage
    : public ClusterAggregateComp<Image, ImageBlock, long, Image> {

public:
  ENABLE_DEEP_COPY

  int x;
  int y;
  int z;

  ConvChunksToImage() {}

  ConvChunksToImage(int x, int y, int z) : x(x), y(y), z(z) {}

  // the key type must have == and size_t hash () defined
  Lambda<long> getKeyProjection(Handle<ImageBlock> aggMe) override {
    return makeLambda(aggMe, [this](Handle<ImageBlock> &aggMe) {
      return (long) aggMe->block_x_index;
    });
  }

  // the value type must have + defined
  Lambda<Image> getValueProjection(Handle<ImageBlock> aggMe) override {
    return makeLambda(aggMe, [this](Handle<ImageBlock> &aggMe) {
      Handle<Image> temp = makeObject<Image>(aggMe->block_x_index, x, y, z);
      Handle<FFMatrixBlock> channel = makeObject<FFMatrixBlock>(0, 0, x, y, x, y);
      double *data = channel->getValue().rawData->c_ptr();

      Map<int, Vector<double>> &chunks = aggMe->getBlock();
      int i = 0;
      while (i < x * y) {
        assert(chunks.count(i) != 0);
        Vector<double> &in = chunks[i];

        for(int j = 0; j < in.size(); j++) {
          data[i + j] = in[j];
        }

        i += in.size();
      }

      for (int c = 0; c < z; c++) {
        if (c == aggMe->block_y_index)
          temp->addChannel(channel);
        else
          temp->addChannel(nullptr);
      }

      return *temp;
    });
  }
};

} // namespace conv2d_memory_fusion

#endif
