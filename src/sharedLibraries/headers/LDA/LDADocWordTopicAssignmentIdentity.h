#ifndef LDA_DOC_WORD_TOPIC_IDENT_H
#define LDA_DOC_WORD_TOPIC_IDENT_H

#include "SelectionComp.h"
#include "Lambda.h"
#include "LDADocWordTopicAssignment.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

class LDADocWordTopicAssignmentIdentity
    : public SelectionComp<LDADocWordTopicAssignment, LDADocWordTopicAssignment> {

public:
    ENABLE_DEEP_COPY

    LDADocWordTopicAssignmentIdentity() {}

    Lambda<bool> getSelection(Handle<LDADocWordTopicAssignment> in) override {
        return makeLambda(in, [&](Handle<LDADocWordTopicAssignment>& in) { return true; });
    }

    Lambda<Handle<LDADocWordTopicAssignment>> getProjection(
        Handle<LDADocWordTopicAssignment> in) override {
        return makeLambda(in, [&](Handle<LDADocWordTopicAssignment>& in) { return in; });
    }
};

#endif
