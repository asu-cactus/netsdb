#ifndef COMMENT_BLOCKS_PARTITION_H
#define COMMENT_BLOCKS_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "FFMatrixBlock.h"
#include "PartitionComp.h"

using namespace pdb;

enum class MatrixBlockPartitionType { Row = 1, Column };

namespace reddit {
class MatrixBlockPartition : public PartitionComp<int, ::FFMatrixBlock> {
public:
  ENABLE_DEEP_COPY

  MatrixBlockPartitionType partition_type;

  MatrixBlockPartition() : partition_type(MatrixBlockPartitionType::Row) {}


  MatrixBlockPartition(std::string dbname, std::string setname)
      : partition_type(MatrixBlockPartitionType::Row) {
    this->setOutput(dbname, setname);
  }

  MatrixBlockPartition(std::string dbname, std::string setname,
                       MatrixBlockPartitionType partition_type)
      : partition_type(partition_type) {

    this->setOutput(dbname, setname);
  }

  Lambda<int> getProjection(Handle<::FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe, [this](Handle<::FFMatrixBlock> &checkMe) {
      if (partition_type == MatrixBlockPartitionType::Row)
        return checkMe->getBlockRowIndex();
      else
        return checkMe->getBlockColIndex();
    });
  }
};
} // namespace reddit

#endif