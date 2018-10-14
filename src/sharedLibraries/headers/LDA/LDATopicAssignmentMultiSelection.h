#ifndef LDA_TOPIC_ASSIGN_MULTI_SELECT_H
#define LDA_TOPIC_ASSIGN_MULTI_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "MultiSelectionComp.h"
#include "PDBVector.h"
#include "LDA/LDADocWordTopicCount.h"
#include "LDADocWordTopicAssignment.h"

/* This class implements a multi-selection that extracts topic assignment from LDADocWordTopicAssignment */
using namespace pdb;
class LDATopicAssignmentMultiSelection
    : public MultiSelectionComp<TopicAssignment, LDADocWordTopicAssignment> {

public:
    ENABLE_DEEP_COPY

    Lambda<bool> getSelection(Handle<LDADocWordTopicAssignment> checkMe) override {
        return makeLambda(checkMe, [](Handle<LDADocWordTopicAssignment>& checkMe) { return true; });
    }

    Lambda<Vector<Handle<TopicAssignment>>> getProjection(
        Handle<LDADocWordTopicAssignment> checkMe) override {
        return makeLambdaFromMethod(checkMe, getTopicAssigns);
    }
};


#endif
