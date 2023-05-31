#ifndef ENSEMBLE_TREE_COMPILED_UDF_FLOAT_H
#define ENSEMBLE_TREE_COMPILED_UDF_FLOAT_H


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

// PRELOAD %EnsembleTreeCompiledUDFFloat%


namespace pdb 
{
    class EnsembleTreeCompiledUDFFloat : public SelectionComp<Vector<float>, TensorBlock2D<float>>
    {

    public:
        ENABLE_DEEP_COPY

	void (*func_name) (float * input, float * output, size_t numRows, size_t numCols);

        EnsembleTreeCompiledUDFFloat() {}

        EnsembleTreeCompiledUDFFloat(void (*func_name) (float * input, float * output, size_t numRows, size_t numCols))
        {
            this->func_name = func_name;
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
                Handle<Vector<float>> resultMatrix  = nullptr;

		TensorBlock2D<float> & block = *in;

		// get the input array of floats
                float * input = (block.data.rawData)->c_ptr();
            
	        size_t numRows =  block.getRowNums();
		size_t numCols =  block.getColNums();

                // set the output matrix
                resultMatrix = makeObject<pdb::Vector<float>>(numRows, numRows);
		float * output = resultMatrix->c_ptr();
		func_name(input, output, numRows, numCols);

                return resultMatrix; });
        }
    };
}

#endif
