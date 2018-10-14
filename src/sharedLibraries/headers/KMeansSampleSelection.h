
#ifndef K_MEANS_SAMPLE_SELECTION_H
#define K_MEANS_SAMPLE_SELECTION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "KMeansDoubleVector.h"
#include "PDBVector.h"
#include "PDBString.h"
#include <cstdlib>

/* This class assigns random values to the input KMeansDoubleVector */ 
using namespace pdb;
class KMeansSampleSelection : public SelectionComp<KMeansDoubleVector, KMeansDoubleVector> {

private:
    double fraction;

public:
    ENABLE_DEEP_COPY

    KMeansSampleSelection() {}

    KMeansSampleSelection(double inputFraction) {
        this->fraction = inputFraction;
    }

    /* 
     * Assign random values to checkMe
     * srand has already been invoked in server
     */
    Lambda<bool> getSelection(Handle<KMeansDoubleVector> checkMe) override {
        return makeLambda(checkMe, [&](Handle<KMeansDoubleVector>& checkMe) {
            double myVal = (double)rand() / (double)RAND_MAX;
            bool ifSample = (myVal <= (this->fraction));
            if (ifSample)
                return true;
            else
                return false;
        });
    }

    Lambda<Handle<KMeansDoubleVector>> getProjection(Handle<KMeansDoubleVector> checkMe) override {
        return makeLambda(checkMe, [](Handle<KMeansDoubleVector>& checkMe) { return checkMe; });
    }
};


#endif
