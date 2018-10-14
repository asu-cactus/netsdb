#ifndef LDA_DOC_WORD_TOPIC_ASSIGNMENT_H
#define LDA_DOC_WORD_TOPIC_ASSIGNMENT_H

#include "Object.h"
#include "PDBVector.h"
#include "Handle.h"
#include "LDA/TopicAssignment.h"
#include "LDA/DocAssignment.h"

/* This class stores the pair (DocAssighment, TopicAssignment) */
using namespace pdb;

class LDADocWordTopicAssignment : public Object {

private:
    Handle<Vector<Handle<DocAssignment>>> myDocsAssigns;
    Handle<Vector<Handle<TopicAssignment>>> myTopicAssigns;

public:
    ENABLE_DEEP_COPY

    LDADocWordTopicAssignment() {}

    void setup() {
        myDocsAssigns = makeObject<Vector<Handle<DocAssignment>>>();
        myTopicAssigns = makeObject<Vector<Handle<TopicAssignment>>>();
    }

    Vector<Handle<DocAssignment>>& getDocAssigns() {
        return *myDocsAssigns;
    }

    Vector<Handle<TopicAssignment>>& getTopicAssigns() {
        return *myTopicAssigns;
    }

    void push_back(Handle<DocAssignment>& me) {
        myDocsAssigns->push_back(me);
    }

    void push_back(Handle<TopicAssignment>& me) {
        myTopicAssigns->push_back(me);
    }

    ~LDADocWordTopicAssignment() {}
};


#endif
