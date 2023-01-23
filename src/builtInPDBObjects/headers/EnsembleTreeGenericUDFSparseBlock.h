#ifndef ENSEMBLE_TREE_GENERIC_UDF_SPARSE_BLOCK_H
#define ENSEMBLE_TREE_GENERIC_UDF_SPARSE_BLOCK_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "Forest.h"
#include "Handle.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "Object.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "SelectionComp.h"
#include "SparseMatrixBlock.h"

// PRELOAD %EnsembleTreeGenericUDFSparseBlock%

namespace pdb {
class EnsembleTreeGenericUDFSparseBlock : public SelectionComp<Vector<float>, SparseMatrixBlock> {

  public:
    ENABLE_DEEP_COPY

    Handle<pdb::Forest> forest;

    EnsembleTreeGenericUDFSparseBlock() {}

    EnsembleTreeGenericUDFSparseBlock(std::string forestPathIn, ModelType modelType, bool isClassificationTask) {
        forest = makeObject<pdb::Forest>(forestPathIn, modelType, isClassificationTask);
    }

    Lambda<bool> getSelection(Handle<SparseMatrixBlock> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<SparseMatrixBlock> &checkMe) { return true; });
    }

    Lambda<Handle<Vector<float>>> getProjection(Handle<SparseMatrixBlock> in) override {
        return makeLambda(in, [this](Handle<SparseMatrixBlock> &in) {
            Handle<Vector<float>> result = forest->predictSparseBlock(in);
            return result; });
    }
};
} // namespace pdb

#endif
