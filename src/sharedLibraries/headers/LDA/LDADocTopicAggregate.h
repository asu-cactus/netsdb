#ifndef LDA_DOC_TOPIC_AGGREGATE_H
#define LDA_DOC_TOPIC_AGGREGATE_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "IntIntVectorPair.h"
#include "LDADocWordTopicAssignment.h"
#include "PDBVector.h"

using namespace pdb;

/* Aggregate for doc assignment */
class LDADocTopicAggregate
    : public ClusterAggregateComp<DocAssignment, DocAssignment, unsigned, DocAssignment> {

public:
    ENABLE_DEEP_COPY

    Lambda<unsigned> getKeyProjection(Handle<DocAssignment> aggMe) override {
        return makeLambdaFromMethod(aggMe, getKey);
    }

    Lambda<DocAssignment> getValueProjection(Handle<DocAssignment> aggMe) override {
        return makeLambdaFromMethod(aggMe, getValue);
    }
};


#endif
