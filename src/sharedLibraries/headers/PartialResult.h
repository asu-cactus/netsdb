#ifndef PARTIAL_RESULT_H
#define PARTIAL_RESULT_H

#include "Object.h"
#include "Centroid.h"
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <iostream>


#ifndef NUM_CLUSTERS
#define NUM_CLUSTERS 10
#endif


class PartialResult : public pdb::Object {

public:
    ENABLE_DEEP_COPY

    PartialResult() {}


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

private:
    Centroid centroids[NUM_CLUSTERS];
};


#endif
