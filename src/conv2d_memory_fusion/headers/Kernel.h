#ifndef CONV2D_KERNEL_H
#define CONV2D_KERNEL_H

#include "Matrix3D.h"
#include "PDBObject.h"

namespace conv2d_memory_fusion {
class Kernel : public Matrix3D {
public:
  int index;

  ENABLE_DEEP_COPY
  Kernel() {}
  Kernel(int index, int x, int y, int z) : index(index), Matrix3D(x, y, z) {}

  int getKey() { return index; }

  int get_num_channels() { return z; }
};
} // namespace conv2d_memory_fusion

#endif