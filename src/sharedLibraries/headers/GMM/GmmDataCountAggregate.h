#ifndef GMM_DATA_COUNT_AGGREGATE_H
#define GMM_DATA_COUNT_AGGREGATE_H

#include "ClusterAggregateComp.h"
#include "DoubleVector.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SumResult.h"
#include "limits.h"

using namespace pdb;

// GmmDataCountAggregate can be used to count the size of the input dataset
class GmmDataCountAggregate
    : public ClusterAggregateComp<SumResult, DoubleVector, int, int> {

public:
  ENABLE_DEEP_COPY

  GmmDataCountAggregate() {}

  // the key type must have == and size_t hash () defined
  Lambda<int> getKeyProjection(Handle<DoubleVector> aggMe) override {
    return makeLambda(aggMe, [](Handle<DoubleVector> &aggMe) { return 0; });
  }

  // the value type must have + defined
  Lambda<int> getValueProjection(Handle<DoubleVector> aggMe) override {
    return makeLambda(aggMe, [](Handle<DoubleVector> &aggMe) { return 1; });
  }
};

#endif
