#ifndef SILLY_LA_MINELEMENT_AGGREGATE_H
#define SILLY_LA_MINELEMENT_AGGREGATE_H

// by Binhang, May 2017

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "LAMinElementValueType.h"
#include "LAMinElementOutputType.h"
#include "MatrixBlock.h"


using namespace pdb;

class LASillyMinElementAggregate
    : public ClusterAggregateComp<LAMinElementOutputType, MatrixBlock, int, LAMinElementValueType> {

public:
    ENABLE_DEEP_COPY

    LASillyMinElementAggregate() {}

    // the key type must have == and size_t hash () defined
    Lambda<int> getKeyProjection(Handle<MatrixBlock> aggMe) override {
        return makeLambda(aggMe, [](Handle<MatrixBlock>& aggMe) { return 1; });
    }

    // the value type must have + defined
    Lambda<LAMinElementValueType> getValueProjection(Handle<MatrixBlock> aggMe) override {
        return makeLambdaFromMethod(aggMe, getMinElementValue);
    }
};


#endif
