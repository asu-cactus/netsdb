#ifndef ENSEMBLE_TREE_GENERIC_UDF_SPARSE_H
#define ENSEMBLE_TREE_GENERIC_UDF_SPARSE_H

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
#include "TensorBlock2D.h"

// PRELOAD %EnsembleTreeGenericUDFSparse%

namespace pdb {
class EnsembleTreeGenericUDFSparse : public SelectionComp<float, Map<int, float>> {

  public:
    ENABLE_DEEP_COPY

    Handle<pdb::Forest> forest;

    EnsembleTreeGenericUDFSparse() {}

    EnsembleTreeGenericUDFSparse(std::string forestPathIn, ModelType modelType, bool isClassificationTask) {
        forest = makeObject<pdb::Forest>(forestPathIn, modelType, isClassificationTask);
    }

    Lambda<bool> getSelection(Handle<Map<int, float>> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<Map<int, float>> &checkMe) { return true; });
    }

    Lambda<Handle<float>> getProjection(Handle<Map<int, float>> in) override {
        return makeLambda(in, [this](Handle<Map<int, float>> &in) {
            // set the output matrix
            Handle<float> result = forest->predictSparse(in);
            return result; });
    }
};
} // namespace pdb

#endif
