#ifndef INFERENCE_RESULT_H
#define INFERENCE_RESULT_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include "FFMatrixBlock.h"
#include "FFMatrixData.h"

#include "PDBVector.h"

#include <cassert>

using namespace pdb;

using namespace std;

class InferenceResult : public Object {
public:
  int index;
  int block_row_id;

  Handle<Vector<double>> inference = nullptr;

  ENABLE_DEEP_COPY

  // default constructor
  InferenceResult() {}

  InferenceResult(int index, int block_row_id)
      : index(index), block_row_id(block_row_id) {
    inference = makeObject<Vector<double>>();
  }

  int getKey() { return this->index; }

  Vector<double> &getInference() { return (*inference); }

  int getLabel() { return (*inference)[0] > (*inference)[1] ? 1 : -1; }

  void dump() {
    std::cout << (*inference)[0] << ", " << (*inference)[1] << std::endl;
  }

  InferenceResult &getValue() { return *this; }
};

#endif
