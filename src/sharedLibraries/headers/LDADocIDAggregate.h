#ifndef LDA_DOC_ID_AGGREGATE_H
#define LDA_DOC_ID_AGGREGATE_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "limits.h"
#include "SumResult.h"
#include "LDADocument.h"

/* The class for extracting document IDs */
using namespace pdb;

class LDADocIDAggregate : public ClusterAggregateComp<SumResult, LDADocument, int, int> {

public:
    ENABLE_DEEP_COPY

    LDADocIDAggregate() {}

    Lambda<int> getKeyProjection(Handle<LDADocument> aggMe) override {
        return makeLambda(aggMe, [](Handle<LDADocument>& aggMe) { return (int)aggMe->getDoc(); });
    }

    Lambda<int> getValueProjection(Handle<LDADocument> aggMe) override {
        return makeLambda(aggMe, [](Handle<LDADocument>& aggMe) { return 1; });
    }
};


#endif
