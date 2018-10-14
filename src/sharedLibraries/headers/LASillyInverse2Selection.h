#ifndef SILLY_LA_INVERSE2_SELECT_H
#define SILLY_LA_INVERSE2_SELECT_H

// by Binhang, June 2017

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "LASingleMatrix.h"

// LA libraries:
#include <eigen3/Eigen/Dense>

using namespace pdb;

class LASillyInverse2Selection : public SelectionComp<SingleMatrix, SingleMatrix> {

public:
    ENABLE_DEEP_COPY

    LASillyInverse2Selection() {}

    Lambda<bool> getSelection(Handle<SingleMatrix> checkMe) override {
        return makeLambda(checkMe, [](Handle<SingleMatrix>& checkMe) { return true; });
    }


    Lambda<Handle<SingleMatrix>> getProjection(Handle<SingleMatrix> checkMe) override {
        return makeLambdaFromMethod(checkMe, getInverse);
    }
};


#endif
