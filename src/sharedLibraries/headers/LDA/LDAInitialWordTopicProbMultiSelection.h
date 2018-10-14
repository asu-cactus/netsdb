
#ifndef LDA_INITIAL_WORD_TOPIC_PROB_MULTI_SELECT_H
#define LDA_INITIAL_WORD_TOPIC_PROB_MULTI_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "MultiSelectionComp.h"
#include "PDBVector.h"
#include "IntDoubleVectorPair.h"

using namespace pdb;
class LDAInitialWordTopicProbMultiSelection : public MultiSelectionComp<IntDoubleVectorPair, int> {

private:
    int numTopic;

public:
    ENABLE_DEEP_COPY

    LDAInitialWordTopicProbMultiSelection() {}
    LDAInitialWordTopicProbMultiSelection(int fromTopic) {
        this->numTopic = fromTopic;
    }

    Lambda<bool> getSelection(Handle<int> checkMe) override {
        return makeLambda(checkMe, [](Handle<int>& checkMe) { return true; });
    }

    Lambda<Vector<Handle<IntDoubleVectorPair>>> getProjection(Handle<int> checkMe) override {
        return makeLambda(checkMe, [&](Handle<int>& checkMe) {
            int numWord = *checkMe;
            Handle<Vector<Handle<IntDoubleVectorPair>>> result =
                makeObject<Vector<Handle<IntDoubleVectorPair>>>();
            for (int i = 0; i < numWord; i++) {
                Handle<IntDoubleVectorPair> wordTopicProb = makeObject<IntDoubleVectorPair>();
                wordTopicProb->myInt = i;
                for (int j = 0; j < numTopic; j++) {
                    wordTopicProb->myVector.push_back(1.0);
                    //     wordTopicProb->myVector.print();
                }
                result->push_back(wordTopicProb);
                //	std::cout << "In multi-selection, the topic probability for word " << i << " : "
                //<< std::endl;
                //	((*result)[i])->getVector().print();
            }

            return *result;
        });
    }
};


#endif
