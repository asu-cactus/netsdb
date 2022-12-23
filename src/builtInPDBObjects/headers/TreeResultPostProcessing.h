#ifndef TREERESULTPOSTPROCESSING_H
#define TREERESULTPOSTPROCESSING_H

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

#include "Handle.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "Object.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "SelectionComp.h"
#include "TensorBlock2D.h"
#include "TreeResult.h"

// PRELOAD %TreeResultPostProcessing%

namespace pdb {
class TreeResultPostProcessing : public SelectionComp<TreeResult, TreeResult> {

  public:
    ENABLE_DEEP_COPY

    int numTrees = 0;
    bool isClassification;

    TreeResultPostProcessing() {}

    TreeResultPostProcessing(int numTrees, bool isClassification)
        : numTrees{numTrees}, isClassification{isClassification} {
        std::cout << "TreeResultPostProcessing is initialized to have " << numTrees
                  << " trees. isClassification=" << (isClassification ? "True\n" : "False\n");
    }

    Lambda<bool> getSelection(Handle<TreeResult> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<TreeResult> &checkMe) { return true; });
    }

    Lambda<Handle<TreeResult>> getProjection(Handle<TreeResult> in) override {
        return makeLambda(in, [this](Handle<TreeResult> &in) {
		    Handle<TreeResult> out = makeObject<TreeResult>(0, in->batchId, in->blockSize, in->modelType);
            in->postprocessing(out, isClassification, numTrees); 
            return out; });
    }
};
} // namespace pdb

#endif
