#ifndef CONV2D_MATRIX3D_H
#define CONV2D_MATRIX3D_H

#include "FFMatrixBlock.h"
#include "Handle.h"
#include "PDBObject.h"
#include "PDBVector.h"

#include <cassert>

namespace conv2d_memory_fusion {
class Matrix3D : public pdb::Object {
public:

  int x;
  int y;
  int z;
  pdb::Vector<pdb::Handle<FFMatrixBlock>> matrices; // size z

  ENABLE_DEEP_COPY
  Matrix3D() {}
  Matrix3D(int x, int y, int z) : x(x), y(y), z(z) { matrices.resize(z); }

  void addChannel(pdb::Handle<FFMatrixBlock> data) {
    matrices.push_back(data);
  }

  void setMatrixAtIndex(int i, pdb::Handle<FFMatrixBlock> data) {
    assert(i < z);
    matrices[i] = data;
  }

  pdb::Handle<FFMatrixBlock> getMatrixAtIndex(int i) {
    assert(i < z);
    return matrices[i];
  }
};
} // namespace conv2d_memory_fusion

#endif
