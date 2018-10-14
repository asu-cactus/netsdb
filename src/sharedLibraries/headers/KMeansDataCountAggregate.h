#ifndef K_MEANS_DATA_COUNT_AGGREGATE_H
#define K_MEANS_DATA_COUNT_AGGREGATE_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "KMeansDoubleVector.h"
#include "limits.h"
#include "SumResult.h"

/* The aggregation that counts the total number of data points */
using namespace pdb;

class KMeansDataCountAggregate
    : public ClusterAggregateComp<SumResult, KMeansDoubleVector, int, int> {

public:
    ENABLE_DEEP_COPY

    KMeansDataCountAggregate() {}

    Lambda<int> getKeyProjection(Handle<KMeansDoubleVector> aggMe) override {
        return makeLambda(aggMe, [](Handle<KMeansDoubleVector>& aggMe) { return 0; });
    }

    Lambda<int> getValueProjection(Handle<KMeansDoubleVector> aggMe) override {
        return makeLambda(aggMe, [](Handle<KMeansDoubleVector>& aggMe) { return 1; });
    }
};


#endif
