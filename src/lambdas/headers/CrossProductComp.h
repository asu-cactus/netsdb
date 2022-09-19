
#ifndef CROSS_PRODUCT_COMP
#define CROSS_PRODUCT_COMP

#include "JoinComp.h"
#include "PDBVector.h"
#include "Tree.h"
#include "TreeResult.h"
#include "TensorBlock2D.h"
#include "LambdaCreationFunctions.h"

namespace pdb {

class CrossProductComp : public JoinComp<TreeResult, Tree, TensorBlock2D<float>> {

public:



    // the computation returned by this method is called to see if a data item should be returned in
    // the output set
    virtual Lambda<bool> getSelection(Handle<Tree> in1,
                                      Handle<TensorBlock2D<float>> in2) override{
        return makeLambda(in1, in2, [](Handle<Tree>& in1, Handle<TensorBlock2D<float>>& in2) { return true; });
    }
 

    // the computation returned by this method is called to produce output tuples from this method
    virtual Lambda<Handle<TreeResult>> getProjection(Handle<Tree> in1,
                                              Handle<TensorBlock2D<float>> in2) override = 0;

protected:

    JoinType joinType = CrossProduct;

};
}

#endif
