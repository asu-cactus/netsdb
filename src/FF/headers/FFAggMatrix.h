#ifndef FF_AGG_MATRIX_H
#define FF_AGG_MATRIX_H

#include "ClusterAggregateComp.h"
#include "FFMatrixBlock.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

// This aggregation will
class FFAggMatrix : public ClusterAggregateComp<FFMatrixBlock, FFMatrixBlock,
                                                MatrixMeta, FFMatrixData> {

public:
  ENABLE_DEEP_COPY

  FFAggMatrix() {}

  // the key type must have == and size_t hash () defined
  Lambda<MatrixMeta> getKeyProjection(Handle<FFMatrixBlock> aggMe) override {
      return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe) {
          return aggMe->getKey();
      });
    // return makeLambdaFromMethod(aggMe, getKey);
  }

  // the value type must have + defined
  Lambda<FFMatrixData>
  getValueProjection(Handle<FFMatrixBlock> aggMe) override {
      return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe) {
          return aggMe->getValue();
      });
    // return makeLambdaFromMethod(aggMe, getValue);
  }
};

#endif