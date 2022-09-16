#ifndef TREE_CROSS_PRODUCT_H
#define TREE_CROSS_PRODUCT_H


#include "CrossProductComp.h"
#include "PDBString.h"
#include "StringIntPair.h"
#include "LambdaCreationFunctions.h"
#include "TensorBlock2D.h"
#include "Tree.h"
#include "TreeResult.h"

// PRELOAD %TreeCrossProduct%

using namespace pdb;

class TreeCrossProduct : public CrossProductComp {

public:
    ENABLE_DEEP_COPY

    TreeCrossProduct() {}


    Lambda<Handle<TreeResult>> getProjection(Handle<Tree> in1, Handle<TensorBlock2D<float>> in2) override {
        return makeLambda(in1, in2, [](Handle<Tree>& in1, Handle<TensorBlock2D<float>>& in2) {
            Handle<TreeResult> result = in1->predict(in2);
            return result;
        });
    }
};


#endif
