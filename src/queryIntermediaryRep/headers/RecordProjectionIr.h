#ifndef PDB_QUERYINTERMEDIARYREP_RECORDPROJECTIONIR_H
#define PDB_QUERYINTERMEDIARYREP_RECORDPROJECTIONIR_H

#include "Handle.h"
#include "Lambda.h"
#include "QueryNodeIr.h"
#include "QueryNodeIrAlgo.h"

using pdb::Handle;
using pdb::Lambda;
using pdb::Object;

namespace pdb_detail {
class RecordProjectionIr {

public:
    virtual Lambda<Handle<Object>> toLambda(Handle<Object>& inputRecordPlaceholder) = 0;
};
}

#endif  // PDB_QUERYINTERMEDIARYREP_RECORDPROJECTIONIR_H
