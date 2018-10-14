#ifndef LDA_WORD_TOPIC_AGGREGATE_H
#define LDA_WORD_TOPIC_AGGREGATE_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "IntDoubleVectorPair.h"
#include "LDATopicWordProb.h"

/* Aggregation for LDATopicWordProb */
using namespace pdb;

class LDAWordTopicAggregate
    : public ClusterAggregateComp<LDATopicWordProb, LDATopicWordProb, unsigned, LDATopicWordProb> {

public:
    ENABLE_DEEP_COPY

    Lambda<unsigned> getKeyProjection(Handle<LDATopicWordProb> aggMe) override {
        return makeLambdaFromMethod(aggMe, getKey);
    }

    Lambda<LDATopicWordProb> getValueProjection(Handle<LDATopicWordProb> aggMe) override {
        return makeLambdaFromMethod(aggMe, getValue);
    }
};


#endif
