#ifndef FF_ROW_AGGREGATE_H
#define FF_ROW_AGGREGATE_H

#include "ClusterAggregateComp.h"
#include "FFMatrixBlock.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

// This aggregation will
class FFRowAggregate : public ClusterAggregateComp<FFMatrixBlock, FFMatrixBlock,
                                                   Handle<FFMatrixMeta>, FFMatrixData> {

public:
  ENABLE_DEEP_COPY

  FFRowAggregate() {}

  // the key type must have == and size_t hash () defined
  Lambda<Handle<FFMatrixMeta>> getKeyProjection(Handle<FFMatrixBlock> aggMe) override {
    return makeLambdaFromMethod(aggMe, getRowKey);
  }

  // the value type must have + defined
  Lambda<FFMatrixData>
  getValueProjection(Handle<FFMatrixBlock> aggMe) override {
    return makeLambdaFromMethod(aggMe, getRowSumValue);
  }
};

#endif
