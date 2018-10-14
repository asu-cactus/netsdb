
#ifndef DOC_ASSIGNMENT_H
#define DOC_ASSIGNMENT_H

#include "Object.h"
#include "Handle.h"
#include "NumericalVector.h"

/* This class stores the topic assignment for a document */
using namespace pdb;

class DocAssignment : public Object {

    unsigned whichDoc = 0;
    NumericalVector<unsigned> whichTopics;

public:
    ENABLE_DEEP_COPY

    DocAssignment(unsigned numDims, unsigned whichDoc, unsigned whichTopic, unsigned cnt)
        : whichDoc(whichDoc), whichTopics(numDims, whichTopic, cnt) {}

    DocAssignment() {}

    DocAssignment& operator+(DocAssignment& addMe) {
        whichTopics += addMe.whichTopics;
        return *this;
    }

    unsigned& getKey() {
        return whichDoc;
    }

    DocAssignment& getValue() {
        return *this;
    }

    Vector<unsigned>& getVector() {
        return whichTopics.getVector();
    }
};

#endif
