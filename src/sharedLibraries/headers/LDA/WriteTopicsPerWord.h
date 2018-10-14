#ifndef WRITE_TOPICS_PER_WORD_H
#define WRITE_TOPICS_PER_WORD_H

#include "WriteUserSet.h"
#include "LDATopicWordProb.h"

using namespace pdb;
class WriteTopicsPerWord : public WriteUserSet<LDATopicWordProb> {

public:
    ENABLE_DEEP_COPY

    WriteTopicsPerWord() {}

    WriteTopicsPerWord(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
