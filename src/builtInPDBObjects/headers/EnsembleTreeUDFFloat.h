#ifndef ENSEMBLE_TREE_UDF_FLOAT_H
#define ENSEMBLE_TREE_UDF_FLOAT_H


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
#include "ForestObjectBased.h"


// PRELOAD %EnsembleTreeUDFFloat%


namespace pdb 
{
    class EnsembleTreeUDFFloat : public SelectionComp<Vector<float>, TensorBlock2D<float>>
    {

    public:
        ENABLE_DEEP_COPY

	Handle<ForestObjectBased> forest = nullptr;

        EnsembleTreeUDFFloat() {}

        EnsembleTreeUDFFloat(std::string forestPathIn, ModelType modelType, bool isClassificationTask)
        {
            Handle<pdb::Forest> forestIn = makeObject<pdb::Forest>(forestPathIn, modelType, isClassificationTask);
	    forest = makeObject<pdb::ForestObjectBased>(forestIn);
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
