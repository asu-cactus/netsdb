#ifndef LDA_INITIAL_WORD_TOPIC_PROB_SELECT_H
#define LDA_INITIAL_WORD_TOPIC_PROB_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "PDBVector.h"
#include "IntDoubleVectorPair.h"
#include "LDA/LDATopicWordProb.h"

/* This class initialize the topic-probability for each word */
using namespace pdb;
class LDAInitialWordTopicProbSelection : public SelectionComp<LDATopicWordProb, int> {

private:
    unsigned numTopic;

public:
    ENABLE_DEEP_COPY

    LDAInitialWordTopicProbSelection() {}
    LDAInitialWordTopicProbSelection(unsigned numTopicIn) {
        this->numTopic = numTopicIn;
    }

    Lambda<bool> getSelection(Handle<int> checkMe) override {
        return makeLambda(checkMe, [](Handle<int>& checkMe) { return true; });
    }

    Lambda<Handle<LDATopicWordProb>> getProjection(Handle<int> checkMe) override {

        return makeLambda(checkMe, [&](Handle<int>& checkMe) {

            int numWord = *checkMe;
            srand48(numWord);

            Handle<Vector<double>> wordTopicProb = makeObject<Vector<double>>(numTopic);
            for (int j = 0; j < numTopic; j++) {
                wordTopicProb->push_back(drand48());
            }
            Handle<LDATopicWordProb> myList = makeObject<LDATopicWordProb>(numWord, wordTopicProb);

            return myList;
        });
    }
};

#endif
