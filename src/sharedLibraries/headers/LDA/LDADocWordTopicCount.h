
#ifndef LDA_DOC_WORD_TOPIC_COUNT_H
#define LDA_DOC_WORD_TOPIC_COUNT_H

#include "Object.h"
#include "Handle.h"

// By Shangyu

using namespace pdb;

class LDADocWordTopicCount : public Object {

private:
    int docID;
    int wordID;
    int topicID;
    int count;

public:
    ENABLE_DEEP_COPY

    ~LDADocWordTopicCount() {}
    LDADocWordTopicCount() {}


    LDADocWordTopicCount(int fromDoc, int fromWord, int fromTopic, int fromCount) {
        this->docID = fromDoc;
        this->wordID = fromWord;
        this->topicID = fromTopic;
        this->count = fromCount;
    }


    int getDoc() {
        return this->docID;
    }

    int getTopic() {
        return this->topicID;
    }

    int getWord() {
        return this->wordID;
    }

    int getCount() {
        return this->count;
    }
};


#endif
