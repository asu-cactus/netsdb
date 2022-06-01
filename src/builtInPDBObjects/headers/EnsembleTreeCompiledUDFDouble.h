#ifndef ENSEMBLE_TREE_COMPILED_UDF_DOUBLE_H
#define ENSEMBLE_TREE_COMPILED_UDF_DOUBLE_H


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

// PRELOAD %EnsembleTreeCompiledUDFDouble%


namespace pdb 
{
    class EnsembleTreeCompiledUDFDouble : public SelectionComp<Vector<double>, TensorBlock2D<double>>
    {

    public:
        ENABLE_DEEP_COPY

	void (*func_name) (double * input, double * output, size_t numRows, size_t numCols);

        EnsembleTreeCompiledUDFDouble() {}

        EnsembleTreeCompiledUDFDouble(void (*func_name) (double * input, double * output, size_t numRows, size_t numCols))
        {
            this->func_name = func_name;
        }

        Lambda<bool> getSelection(Handle<TensorBlock2D<double>> checkMe) override
        {
            return makeLambda(checkMe,
                              [](Handle<TensorBlock2D<double>> &checkMe)
                              { return true; });
        }

        Lambda<Handle<Vector<double>>> getProjection(Handle<TensorBlock2D<double>> in) override
        {
            return makeLambda(in, [this](Handle<TensorBlock2D<double>> &in) {


		TensorBlock2D<double> & block = *in;

		// get the input array of doubles
                double * input = (block.data.rawData)->c_ptr();
            
	        size_t numRows =  block.getRowNums();
		size_t numCols =  block.getColNums();

                // set the output matrix
                Handle<Vector<double>> resultMatrix = makeObject<pdb::Vector<double>>(numRows, numRows);
		double * output = resultMatrix->c_ptr();
		func_name(input, output, numRows, numCols);

                return resultMatrix; });
        }
    };
}

#endif
