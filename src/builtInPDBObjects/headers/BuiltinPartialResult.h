#ifndef BUILTIN_PARTIAL_RESULT_H
#define BUILTIN_PARTIAL_RESULT_H

// PRELOAD %BuiltinPartialResult%
#include "Object.h"
#include "Centroid.h"
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <iostream>


#ifndef NUM_CLUSTERS
#define NUM_CLUSTERS 10
#endif

namespace pdb {
class BuiltinPartialResult : public Object {

public:
    ENABLE_DEEP_COPY

    BuiltinPartialResult() {}
    ~BuiltinPartialResult() {}

    void initialize() {

        int i;
        for (i = 0; i < NUM_CLUSTERS; i++) {
            centroids[i].initialize();
        }
    }


    void updateCentroid(int clusterIndex, double* point) {

        centroids[clusterIndex].update(point);
    }

    void printCentroids() {
        int i;
        std::cout << std::endl;
        for (i = 0; i < NUM_CLUSTERS; i++) {
            std::cout << "cluster-" << i << ":" << std::endl;
            centroids[i].print();
        }
    }

    Centroid* getCentroids() {

        return centroids;
    }


private:
    Centroid centroids[NUM_CLUSTERS];
};
}


#endif
