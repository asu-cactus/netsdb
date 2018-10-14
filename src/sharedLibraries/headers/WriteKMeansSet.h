
#ifndef WRITE_K_MEANS_SET_H
#define WRITE_K_MEANS_SET_H

#include "WriteUserSet.h"
#include "KMeansAggregateOutputType.h"

using namespace pdb;
class WriteKMeansSet : public WriteUserSet<KMeansAggregateOutputType> {

public:
    ENABLE_DEEP_COPY

    WriteKMeansSet() {}

    WriteKMeansSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
