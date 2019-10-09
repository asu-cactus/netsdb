#ifndef SILLY_LA_MULTIPLY2_AGGREGATE_H
#define SILLY_LA_MULTIPLY2_AGGREGATE_H

// by Binhang, May 2017

#include "ClusterAggregateComp.h"
#include "MatrixSparse.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class SparseMultiplyAggregate
    : public ClusterAggregateComp<MatrixSparse,MatrixSparse, MatrixMeta, MatrixData> {

public:
    ENABLE_DEEP_COPY

    SparseMultiplyAggregate() {}

    // the key type must have == and size_t hash () defined
    Lambda<MatrixMeta> getKeyProjection(Handle<MatrixSparse> aggMe) override {
        return makeLambdaFromMethod(aggMe, getMultiplyKey);
    }

    // the value type must have + defined
    Lambda<MatrixData> getValueProjection(Handle<MatrixSparse> aggMe) override {
        return makeLambdaFromMethod(aggMe, getMultiplyValue);
    }
};


#endif
