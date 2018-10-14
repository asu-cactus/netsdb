#ifndef SILLY_LA_COL_SUM_AGGREGATE_H
#define SILLY_LA_COL_SUM_AGGREGATE_H

// by Binhang, May 2017

#include "ClusterAggregateComp.h"
#include "MatrixBlock.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class LASillyColSumAggregate
    : public ClusterAggregateComp<MatrixBlock, MatrixBlock, MatrixMeta, MatrixData> {

public:
    ENABLE_DEEP_COPY

    LASillyColSumAggregate() {}

    // the key type must have == and size_t hash () defined
    Lambda<MatrixMeta> getKeyProjection(Handle<MatrixBlock> aggMe) override {
        return makeLambdaFromMethod(aggMe, getColKey);
    }

    // the value type must have + defined
    Lambda<MatrixData> getValueProjection(Handle<MatrixBlock> aggMe) override {
        return makeLambdaFromMethod(aggMe, getColSumValue);
    }
};


#endif