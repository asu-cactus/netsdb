#ifndef ENSEMBLE_TREE_GENERIC_UDF_H
#define ENSEMBLE_TREE_GENERIC_UDF_H

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
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "Tree.h"
#include "TreeNode.h"
#include "Forest.h"

using namespace pdb;

namespace decisiontree
{
    class EnsembleTreeGenericUDF : public SelectionComp<FFMatrixBlock, FFMatrixBlock>
    {

    public:
        ENABLE_DEEP_COPY

	Handle<Forest> forest;

        EnsembleTreeGenericUDF() {}

        EnsembleTreeGenericUDF(std::string forestPathIn, ModelType modelType, bool isClassificationTask)
        {
            forest = makeObject<Forest>(forestPathIn, modelType, isClassificationTask);
        }

        Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override
        {
            return makeLambda(checkMe,
                              [](Handle<FFMatrixBlock> &checkMe)
                              { return true; });
        }

        Lambda<Handle<FFMatrixBlock>> getProjection(Handle<FFMatrixBlock> in) override
        {
            return makeLambda(in, [this](Handle<FFMatrixBlock> &in) {

                // load the metadata
                uint32_t inNumRow = in->getRowNums();
                uint32_t inNumCol = in->getColNums();
                uint32_t inBlockRowIndex = in->getBlockRowIndex();
                uint32_t inBlockColIndex = in->getBlockColIndex();

                // set the output matrix
                pdb::Handle<pdb::Vector<double>> resultMatrix = pdb::makeObject<pdb::Vector<double>>(); // TODO: Change all Double References to Float
                std::vector<float> thisResultMatrix(forest->numTrees);

                resultMatrix = forest->predict(in);

                pdb::Handle<FFMatrixBlock> resultMatrixBlock = pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, inNumRow, 1, resultMatrix);

                return resultMatrixBlock; });
        }
    };
}

#endif
