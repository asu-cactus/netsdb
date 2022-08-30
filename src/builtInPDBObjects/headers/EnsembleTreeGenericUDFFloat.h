#ifndef ENSEMBLE_TREE_GENERIC_UDF_FLOAT_H
#define ENSEMBLE_TREE_GENERIC_UDF_FLOAT_H


#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <future>
#include <thread>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <memory>
#include <algorithm>
#include <map>
#include <set>
#include <cstring>
#include <exception>

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include "TensorBlock2D.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "Forest.h"


// PRELOAD %EnsembleTreeGenericUDFFloat%


namespace pdb 
{
    class EnsembleTreeGenericUDFFloat : public SelectionComp<Vector<float>, TensorBlock2D<float>>
    {

    public:
        ENABLE_DEEP_COPY

	Handle<pdb::Forest> forest;

        EnsembleTreeGenericUDFFloat() {}

        EnsembleTreeGenericUDFFloat(std::string forestPathIn, ModelType modelType, bool isClassificationTask)
        {
            forest = makeObject<pdb::Forest>(forestPathIn, modelType, isClassificationTask);
        }

        Lambda<bool> getSelection(Handle<TensorBlock2D<float>> checkMe) override
        {
            return makeLambda(checkMe,
                              [](Handle<TensorBlock2D<float>> &checkMe)
                              { return true; });
        }

        Lambda<Handle<Vector<float>>> getProjection(Handle<TensorBlock2D<float>> in) override
        {
            return makeLambda(in, [this](Handle<TensorBlock2D<float>> &in) {


                // set the output matrix
                pdb::Handle<pdb::Vector<float>> resultMatrix = forest->predict(in);

                return resultMatrix; });
        }
    };
}

#endif
