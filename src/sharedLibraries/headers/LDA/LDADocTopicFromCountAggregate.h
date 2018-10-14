#ifndef LDA_DOC_TOPIC_FROM_COUNT_AGGREGATE_H
#define LDA_DOC_TOPIC_FROM_COUNT_AGGREGATE_H

// by Shangyu, May 2017

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "IntIntVectorPair.h"
#include "LDADocWordTopicCount.h"
#include "PDBVector.h"


using namespace pdb;


class LDADocTopicFromCountAggregate
    : public ClusterAggregateComp<IntIntVectorPair, LDADocWordTopicCount, int, Vector<int>> {

private:
    int numTopic;

public:
    ENABLE_DEEP_COPY

    LDADocTopicFromCountAggregate() {}
    LDADocTopicFromCountAggregate(int fromTopic) {
        this->numTopic = fromTopic;
    }


    // the key type must have == and size_t hash () defined
    Lambda<int> getKeyProjection(Handle<LDADocWordTopicCount> aggMe) override {
        return makeLambda(aggMe,
                          [](Handle<LDADocWordTopicCount>& aggMe) { return aggMe->getDoc(); });
    }

    // the value type must have + defined
    Lambda<Vector<int>> getValueProjection(Handle<LDADocWordTopicCount> aggMe) override {
        return makeLambda(aggMe, [&](Handle<LDADocWordTopicCount>& aggMe) {
            Handle<Vector<int>> result = makeObject<Vector<int>>(this->numTopic, this->numTopic);
            result->fill(0);
            (*result)[aggMe->getTopic()] = aggMe->getCount();
            return *result;
        });
    }
};


#endif
