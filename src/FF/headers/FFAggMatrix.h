#ifndef FF_AGG_MATRIX_H
#define FF_AGG_MATRIX_H

#include "ClusterAggregateComp.h"
#include "FFMatrixBlock.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

// This aggregation will
class FFAggMatrix : public ClusterAggregateComp<FFMatrixBlock, FFMatrixBlock,
                                                FFMatrixMeta, FFMatrixData> {

public:
  ENABLE_DEEP_COPY

  FFAggMatrix() {}

  // the key type must have == and size_t hash () defined
  Lambda<FFMatrixMeta> getKeyProjection(Handle<FFMatrixBlock> aggMe) override {
      return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe) {
          return aggMe->getKey();
      });
    // return makeLambdaFromMethod(aggMe, getKey);
  }

  // the value type must have + defined
  Lambda<FFMatrixData>
  getValueProjection(Handle<FFMatrixBlock> aggMe) override {
      return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe) {
          //std::cout << "[FFAggMatrix] " << &aggMe << ", " << &(aggMe->getValue()) << std::endl;
          return aggMe->getValue();
      });
    // return makeLambdaFromMethod(aggMe, getValue);
  }
};

#endif
