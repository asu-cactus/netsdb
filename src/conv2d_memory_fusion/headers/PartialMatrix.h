#ifndef CONV2D_PARTIAL_MATRIX_H
#define CONV2D_PARTIAL_MATRIX_H

#include "Handle.h"
#include "PDBObject.h"
#include "PDBVector.h"

namespace conv2d_memory_fusion {
class PartialMatrix : public pdb::Object {
public:
  int origin_x;
  int origin_y;
  int x;
  int y;
  int key;

  pdb::Vector<float> partial;
  ENABLE_DEEP_COPY

  PartialMatrix() {}

  PartialMatrix(int origin_x, int origin_y, int x, int y, int key)
      : origin_x(origin_x), origin_y(origin_y), x(x), y(y), key(key) {
    partial.resize(x * y);
  }

  // Copy x*y elements from offset
  void copy_partial(pdb::Vector<float> &from, int offset) {
    for (int i = 0; i < x * y; i++) {
      partial[i] = from[offset + i];
    }
  }

  pdb::Vector<float> &getPartial() { return partial; }

  void dump() {
    std::cout << "x: " << x << ", y: " << y << ", origin_x: " << origin_x
              << ", origin_y: " << origin_y << ", key: " << key << std::endl;
    for (int i = 0; i < partial.size(); i++) {
      std::cout << partial[i] << ",";
    }
    std::cout << std::endl;
  }

  int getSize() { return partial.size(); }
};
} // namespace conv2d_memory_fusion

#endif
