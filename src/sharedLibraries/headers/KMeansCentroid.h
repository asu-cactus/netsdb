#ifndef K_MEANS_CENTROID_H
#define K_MEANS_CENTROID_H

#include "Object.h"
#include "Handle.h"
#include "KMeansDoubleVector.h"

/* 
 * This class stores the centroid of a cluster, which contains the number 
 * of data points in this cluster, and the mean of these data points
 */
using namespace pdb;

class KMeansCentroid : public Object {

private:
    KMeansDoubleVector mean;
    size_t count;

public:
    ENABLE_DEEP_COPY

    KMeansCentroid() {}

    KMeansCentroid(size_t count, KMeansDoubleVector value) {
        this->count = count;
        this->mean = value;
    }

    ~KMeansCentroid() {}

    size_t getCount() {
        return this->count;
    }

    KMeansDoubleVector& getMean() {
        return this->mean;
    }

    KMeansCentroid& operator+(KMeansCentroid& other) {
        this->count += other.getCount();
        this->mean = this->mean + other.getMean();
        return *this;
    }
};

#endif
