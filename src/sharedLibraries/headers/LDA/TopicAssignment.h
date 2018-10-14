
#ifndef TOPIC_ASSIGNMENT_H
#define TOPIC_ASSIGNMENT_H

#include "Object.h"
#include "Handle.h"
#include "NumericalVector.h"

/* This class stores the word assignment for a topic */
using namespace pdb;

class TopicAssignment : public Object {

    unsigned whichTopic = 0;
    NumericalVector<unsigned> whichWords;

public:
    ENABLE_DEEP_COPY

    TopicAssignment(unsigned numDims, unsigned whichTopic, unsigned whichWord, unsigned cnt)
        : whichTopic(whichTopic), whichWords(numDims, whichWord, cnt) {}

    TopicAssignment() {}

    TopicAssignment& operator+(TopicAssignment& addMe) {
        whichWords += addMe.whichWords;
        return *this;
    }

    unsigned& getKey() {
        return whichTopic;
    }

    TopicAssignment& getValue() {
        return *this;
    }

    Vector<unsigned>& getVector() {
        return whichWords.getVector();
    }
};

#endif
