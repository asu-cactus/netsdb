#include "MatrixData.h"

class FFMatrixData : public MatrixData {
public:
  ENABLE_DEEP_COPY

  ~FFMatrixData() {}
  FFMatrixData() {}

  pdb::Handle<pdb::Vector<double>> bias;
};