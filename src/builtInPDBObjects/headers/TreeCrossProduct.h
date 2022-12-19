#ifndef TREE_CROSS_PRODUCT_H
#define TREE_CROSS_PRODUCT_H

#include "CrossProductComp.h"
#include "LambdaCreationFunctions.h"
#include "PDBString.h"
#include "StringIntPair.h"
#include "TensorBlock2D.h"
#include "Tree.h"
#include "TreeResult.h"

// PRELOAD %TreeCrossProduct%

using namespace pdb;

class TreeCrossProduct : public CrossProductComp {
  private:
    bool hasMissing;

  public:
    ENABLE_DEEP_COPY

    TreeCrossProduct() : hasMissing{false} { this->joinType = CrossProduct; }
    TreeCrossProduct(bool hasMissing) : hasMissing{hasMissing} { this->joinType = CrossProduct; }

    Lambda<Handle<TreeResult>> getProjection(Handle<Tree> in1, Handle<TensorBlock2D<float>> in2) override {
        if (hasMissing) {
            return makeLambda(in1, in2, [](Handle<Tree> &in1, Handle<TensorBlock2D<float>> &in2) {
                Handle<TreeResult> result = in1->predictWithMissingValues(in2);
                return result;
            });
        } else {
            return makeLambda(in1, in2, [](Handle<Tree> &in1, Handle<TensorBlock2D<float>> &in2) {
                Handle<TreeResult> result = in1->predictWithoutMissingValues(in2);
                return result;
            });
        }
    }
};

#endif
