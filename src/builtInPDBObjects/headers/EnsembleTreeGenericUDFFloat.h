#ifndef ENSEMBLE_TREE_GENERIC_UDF_FLOAT_H
#define ENSEMBLE_TREE_GENERIC_UDF_FLOAT_H

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

// PRELOAD %EnsembleTreeGenericUDFFloat%

namespace pdb {
class EnsembleTreeGenericUDFFloat : public SelectionComp<Vector<float>, TensorBlock2D<float>> {
  private:
    bool hasMissing;

  public:
    ENABLE_DEEP_COPY

    Handle<pdb::Forest> forest;

    EnsembleTreeGenericUDFFloat() : hasMissing{false} {}

    EnsembleTreeGenericUDFFloat(std::string forestPathIn, ModelType modelType, bool isClassificationTask)
        : hasMissing{false} {
        forest = makeObject<pdb::Forest>(forestPathIn, modelType, isClassificationTask);
    }
    EnsembleTreeGenericUDFFloat(std::string forestPathIn, ModelType modelType, bool isClassificationTask, bool hasMissing)
        : hasMissing{hasMissing} {
        forest = makeObject<pdb::Forest>(forestPathIn, modelType, isClassificationTask);
    }
    Lambda<bool> getSelection(Handle<TensorBlock2D<float>> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<TensorBlock2D<float>> &checkMe) { return true; });
    }

    Lambda<Handle<Vector<float>>> getProjection(Handle<TensorBlock2D<float>> in) override {
        if (hasMissing) {
            return makeLambda(in, [this](Handle<TensorBlock2D<float>> &in) {
                // set the output matrix
                pdb::Handle<pdb::Vector<float>> resultMatrix = forest->predictWithMissingValues(in);
                return resultMatrix; });
        } else {
            return makeLambda(in, [this](Handle<TensorBlock2D<float>> &in) {
                // set the output matrix
                pdb::Handle<pdb::Vector<float>> resultMatrix = forest->predictWithoutMissingValues(in);
                return resultMatrix; });
        }
    }
};
} // namespace pdb

#endif
