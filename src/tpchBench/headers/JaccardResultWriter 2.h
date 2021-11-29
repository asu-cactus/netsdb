
#ifndef JACC_RES_WRIT_H
#define JACC_RES_WRIT_H

#include "WriteUserSet.h"
#include "TopJaccard.h"

using namespace pdb;
class JaccardResultWriter : public WriteUserSet<TopKQueue<double, Handle<AllParts>>> {

public:
    ENABLE_DEEP_COPY

    JaccardResultWriter() {}

    JaccardResultWriter(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
