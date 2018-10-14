#ifndef SILLY_LA_MAXELEMENT_AGGREGATE_H
#define SILLY_LA_MAXELEMENT_AGGREGATE_H

// by Binhang, May 2017

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "LAMaxElementValueType.h"
#include "LAMaxElementOutputType.h"
#include "MatrixBlock.h"


using namespace pdb;

class LASillyMaxElementAggregate
    : public ClusterAggregateComp<LAMaxElementOutputType, MatrixBlock, int, LAMaxElementValueType> {

public:
    ENABLE_DEEP_COPY

    LASillyMaxElementAggregate() {}

    // the key type must have == and size_t hash () defined
    Lambda<int> getKeyProjection(Handle<MatrixBlock> aggMe) override {
        return makeLambda(aggMe, [](Handle<MatrixBlock>& aggMe) { return 1; });
    }

    // the value type must have + defined
    Lambda<LAMaxElementValueType> getValueProjection(Handle<MatrixBlock> aggMe) override {
        return makeLambdaFromMethod(aggMe, getMaxElementValue);
    }
};


#endif
