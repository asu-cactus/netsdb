#ifndef PDB_QUERYINTERMEDIARYREP_QUERYGRAPHIR_H
#define PDB_QUERYINTERMEDIARYREP_QUERYGRAPHIR_H

#include "Object.h"
#include "QueryNodeIr.h"
#include "SetExpressionIr.h"

using std::shared_ptr;
using std::vector;

using pdb::Object;

namespace pdb_detail {
class QueryGraphIr {
public:
    QueryGraphIr() {}

    QueryGraphIr(shared_ptr<vector<shared_ptr<SetExpressionIr>>> sinkNodes)
        : _sinkNodes(sinkNodes) {}

    uint32_t getSinkNodeCount() {
        return _sinkNodes->size();
    }

    shared_ptr<SetExpressionIr> getSinkNode(uint32_t index) {
        return _sinkNodes->operator[](index);
    }

private:
    shared_ptr<vector<shared_ptr<SetExpressionIr>>> _sinkNodes;
};

typedef shared_ptr<QueryGraphIr> QueryGraphIrPtr;
}

#endif  // PDB_QUERYINTERMEDIARYREP_QUERYGRAPHIR_H
