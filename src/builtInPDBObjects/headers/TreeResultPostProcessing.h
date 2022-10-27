#ifndef TREERESULTPOSTPROCESSING_H
#define TREERESULTPOSTPROCESSING_H


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
#include "TreeResult.h"


// PRELOAD %TreeResultPostProcessing%


namespace pdb 
{
    class TreeResultPostProcessing : public SelectionComp<TreeResult, TreeResult>
    {

    public:
        ENABLE_DEEP_COPY

        Lambda<bool> getSelection(Handle<TreeResult> checkMe) override
        {
            return makeLambda(checkMe,
                              [](Handle<TreeResult> &checkMe)
                              { return true; });
        }

        Lambda<Handle<TreeResult>> getProjection(Handle<TreeResult> in) override
        {
            return makeLambda(in, [this](Handle<TreeResult> &in) {


		Handle<TreeResult> out = makeObject<TreeResult>();
                in->postprocessing(out); 

                return out; });
        }
    };
}

#endif
