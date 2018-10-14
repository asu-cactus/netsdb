
#ifndef WRITE_LDA_DOC_WORD_TOPIC_ASSIGNMENT_SET_H
#define WRITE_LDA_DOC_WORD_TOPIC_ASSIGNMENT_SET_H

// by Shangyu, June 2017

#include "WriteUserSet.h"
#include "LDADocWordTopicAssignment.h"

using namespace pdb;
class WriteLDADocWordTopicAssignmentSet : public WriteUserSet<LDADocWordTopicAssignment> {

public:
    ENABLE_DEEP_COPY

    WriteLDADocWordTopicAssignmentSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    WriteLDADocWordTopicAssignmentSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
