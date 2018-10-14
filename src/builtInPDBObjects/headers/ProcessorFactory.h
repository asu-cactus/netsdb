//
// Created by barnett on 10/2/16.
//

#ifndef PDB_QUERYINTERMEDIARYREP_PROCESSORFACTORY_H
#define PDB_QUERYINTERMEDIARYREP_PROCESSORFACTORY_H

#include "Object.h"
#include "SimpleSingleTableQueryProcessor.h"

using pdb::Object;
using pdb::SimpleSingleTableQueryProcessorPtr;

namespace pdb {
class ProcessorFactory : public Object {
    virtual SimpleSingleTableQueryProcessorPtr makeProcessor() = 0;
};
}

#endif  // PDB_QUERYINTERMEDIARYREP_PROCESSORFACTORY_H
