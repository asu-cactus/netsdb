#ifndef RF_MATRIX_WRITER_H
#define RF_MATRIX_WRITER_H

#include "WriteUserSet.h"
#include "RFMatrixBlock.h"

using namespace pdb;
class RFMatrixWriter : public WriteUserSet<RFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  RFMatrixWriter() {}

  // below constructor is not required, but if we do not call setOutput() here,
  // we must call setOutput() later to set the output set
  RFMatrixWriter(std::string dbName, std::string setName) {
    this->setOutput(dbName, setName);
  }
};

#endif