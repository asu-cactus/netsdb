#ifndef SCAN_TOPIC_PER_WORD_H
#define SCAN_TOPIC_PER_WORD_H

#include "ScanUserSet.h"
#include "LDATopicWordProb.h"

using namespace pdb;
class ScanTopicsPerWord : public ScanUserSet<LDATopicWordProb> {

public:
    ENABLE_DEEP_COPY

    ScanTopicsPerWord() {}

    ScanTopicsPerWord(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};


#endif
