#ifndef FF_AGG_MATRIX_BY_COL_H
#define FF_AGG_MATRIX_BY_COL_H

#include "ClusterAggregateComp.h"
#include "FFMatrixBlock.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

// This aggregation will
class FFAggMatrixByCol : public ClusterAggregateComp<FFMatrixBlock, FFMatrixBlock,
                                                int, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFAggMatrixByCol() {}

  // the key type must have == and size_t hash () defined
  Lambda<int> getKeyProjection(Handle<FFMatrixBlock> aggMe) override {
    //   return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe) {
    //       return aggMe->getKey();
    //   });
      return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe){return aggMe->getBlockColIndex();});
  }

  // the value type must have + defined
  Lambda<FFMatrixBlock>
  getValueProjection(Handle<FFMatrixBlock> aggMe) override {
      return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe) {
          //std::cout << "[FFAggMatrix] " << &aggMe << ", " << &(aggMe->getValue()) << std::endl;
          return *aggMe;
      });
  }
};

#endif
