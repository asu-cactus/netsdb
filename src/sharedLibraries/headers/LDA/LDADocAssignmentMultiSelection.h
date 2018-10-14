
#ifndef LDA_DOC_ASSIGN_MULTI_SELECT_H
#define LDA_DOC_ASSIGN_MULTI_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "MultiSelectionComp.h"
#include "PDBVector.h"
#include "LDA/LDADocWordTopicCount.h"
#include "LDADocWordTopicAssignment.h"

/* This class implements a multi-selection that extract  doc assignment from LDADocWordTopicAssignment */
using namespace pdb;

class LDADocAssignmentMultiSelection
    : public MultiSelectionComp<DocAssignment, LDADocWordTopicAssignment> {

public:
    ENABLE_DEEP_COPY

    Lambda<bool> getSelection(Handle<LDADocWordTopicAssignment> checkMe) override {
        return makeLambda(checkMe, [](Handle<LDADocWordTopicAssignment>& checkMe) { return true; });
    }

    Lambda<Vector<Handle<DocAssignment>>> getProjection(
        Handle<LDADocWordTopicAssignment> checkMe) override {
        return makeLambdaFromMethod(checkMe, getDocAssigns);
    }
};


#endif
