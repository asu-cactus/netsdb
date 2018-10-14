#ifndef K_MEANS_AGGREGATE_OUTPUT_H
#define K_MEANS_AGGREGATE_OUTPUT_H

#include "Object.h"
#include "Handle.h"
#include "KMeansCentroid.h"

using namespace pdb;

class KMeansAggregateOutputType : public Object {

private:
    int key = 1;
    KMeansCentroid value;

public:
    ENABLE_DEEP_COPY

    KMeansAggregateOutputType() {}

    int& getKey() {
        return key;
    }

    KMeansCentroid& getValue() {
        return value;
    }

    void print() {
        std::cout << "KMeansAggregateOutputType: index: " << key << " value: "
                  << "." << std::endl;
    }

    ~KMeansAggregateOutputType() {}
};

#endif
