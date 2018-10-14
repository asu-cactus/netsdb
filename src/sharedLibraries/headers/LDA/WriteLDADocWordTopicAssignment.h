#ifndef WRITE_DOC_WORD_TOPIC_H
#define WRITE_DOC_WORD_TOPIC_H

#include "WriteUserSet.h"
#include "LDADocWordTopicAssignment.h"

using namespace pdb;
class WriteLDADocWordTopicAssignment : public WriteUserSet<LDADocWordTopicAssignment> {

public:
    ENABLE_DEEP_COPY

    WriteLDADocWordTopicAssignment() {}

    WriteLDADocWordTopicAssignment(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
