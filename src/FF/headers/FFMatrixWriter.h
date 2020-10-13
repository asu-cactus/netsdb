#ifndef FF_MATRIX_WRITER_H
#define FF_MATRIX_WRITER_H

#include "WriteUserSet.h"
#include "FFMatrixBlock.h"

using namespace pdb;
class FFMatrixWriter : public WriteUserSet<FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFMatrixWriter() {}

  // below constructor is not required, but if we do not call setOutput() here,
  // we must call setOutput() later to set the output set
  FFMatrixWriter(std::string dbName, std::string setName) {
    this->setOutput(dbName, setName);
  }
};

#endif
