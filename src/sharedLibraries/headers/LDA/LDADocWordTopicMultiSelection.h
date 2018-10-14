
#ifndef LDA_DOC_WORD_TOPIC_MULTI_SELECT_H
#define LDA_DOC_WORD_TOPIC_MULTI_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "MultiSelectionComp.h"
#include "PDBVector.h"
#include "LDA/LDADocWordTopicCount.h"
#include "LDADocWordTopicAssignment.h"

using namespace pdb;
class LDADocWordTopicMultiSelection
    : public MultiSelectionComp<LDADocWordTopicCount, LDADocWordTopicAssignment> {

public:
    ENABLE_DEEP_COPY

    LDADocWordTopicMultiSelection() {}

    Lambda<bool> getSelection(Handle<LDADocWordTopicAssignment> checkMe) override {
        return makeLambda(checkMe, [](Handle<LDADocWordTopicAssignment>& checkMe) { return true; });
    }

    Lambda<Vector<Handle<LDADocWordTopicCount>>> getProjection(
        Handle<LDADocWordTopicAssignment> checkMe) override {
        return makeLambda(checkMe, [&](Handle<LDADocWordTopicAssignment>& checkMe) {

            Vector<Handle<LDADocWordTopicCount>> result;
            Vector<int>& topicAssign = checkMe->getTopicAssignment();
            int docID = checkMe->getDoc();
            int wordID = checkMe->getWord();

            for (int i = 0; i < topicAssign.size(); i += 2) {
                Handle<LDADocWordTopicCount> myDWTC = makeObject<LDADocWordTopicCount>(
                    docID, wordID, topicAssign[i], topicAssign[i + 1]);
                result.push_back(myDWTC);
            }

            return result;
        });
    }
};


#endif
