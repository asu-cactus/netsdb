#ifndef FF_MATRIX_PARTITION_H
#define FF_MATRIX_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "FFMatrixBlock.h"
#include "PartitionComp.h"

using namespace pdb;

class FFMatrixPartitioner : public PartitionComp<int, FFMatrixBlock> {
public:
  ENABLE_DEEP_COPY

  FFMatrixPartitioner() {}

  FFMatrixPartitioner(std::string dbname, std::string setname) {
    this->setOutput(dbname, setname);
  }

  Lambda<int> getProjection(Handle<FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe, [this](Handle<FFMatrixBlock> &checkMe) {
      return checkMe->getBlockRowIndex();
    });
  }
};

#endif
