#ifndef LDA_TOPIC_WORD_AGGREGATE_H
#define LDA_TOPIC_WORD_AGGREGATE_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "TopicAssignment.h"
#include "ClusterAggregateComp.h"

using namespace pdb;

/* Aggregation for topic assignment */
class LDATopicWordAggregate
    : public ClusterAggregateComp<TopicAssignment, TopicAssignment, unsigned, TopicAssignment> {

public:
    ENABLE_DEEP_COPY

    Lambda<unsigned> getKeyProjection(Handle<TopicAssignment> aggMe) override {
        return makeLambdaFromMethod(aggMe, getKey);
    }

    Lambda<TopicAssignment> getValueProjection(Handle<TopicAssignment> aggMe) override {
        return makeLambdaFromMethod(aggMe, getValue);
    }
};


#endif
