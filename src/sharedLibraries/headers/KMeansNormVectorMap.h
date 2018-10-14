#ifndef K_MEANS_NORM_VECTOR_MAP_H
#define K_MEANS_NORM_VECTOR_MAP_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "KMeansDoubleVector.h"
#include "PDBVector.h"
#include "PDBString.h"
#include <cstdlib>

/* 
 * Use macro for performance consideration 
 * Should be consistent with the value in TestKMeans.cc
 */
#ifndef NUM_KMEANS_DIMENSIONS
#define NUM_KMEANS_DIMENSIONS 1000
#endif

/* This selection computes the 2-norm */
using namespace pdb;
class KMeansNormVectorMap
    : public SelectionComp<KMeansDoubleVector, double[NUM_KMEANS_DIMENSIONS]> {

public:
    ENABLE_DEEP_COPY

    KMeansNormVectorMap() {}

    Lambda<bool> getSelection(Handle<double[NUM_KMEANS_DIMENSIONS]> checkMe) override {
        return makeLambda(checkMe,
                          [&](Handle<double[NUM_KMEANS_DIMENSIONS]>& checkMe) { return true; });
    }

    /* 2-norm computation */
    Lambda<Handle<KMeansDoubleVector>> getProjection(
        Handle<double[NUM_KMEANS_DIMENSIONS]> checkMe) override {
        return makeLambda(checkMe, [](Handle<double[NUM_KMEANS_DIMENSIONS]>& checkMe) {
            Handle<KMeansDoubleVector> ret = makeObject<KMeansDoubleVector>();
            double* rawData = ret->getRawData();
            double* myRawData = *checkMe;
            double norm = 0;
            for (int i = 0; i < NUM_KMEANS_DIMENSIONS; i++) {
                rawData[i] = myRawData[i];
                norm = norm + rawData[i] * rawData[i];
            }
            ret->norm = norm;
            return ret;
        });
    }
};


#endif
