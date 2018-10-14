#ifndef WRITE_KMEANS_DOUBLE_Vector_SET_H
#define WRITE_KMEANS_DOUBLE_Vector_SET_H

#include "WriteUserSet.h"
#include "KMeansDoubleVector.h"

using namespace pdb;
class WriteKMeansDoubleVectorSet : public WriteUserSet<KMeansDoubleVector> {

public:
    ENABLE_DEEP_COPY

    WriteKMeansDoubleVectorSet() {}

    WriteKMeansDoubleVectorSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
