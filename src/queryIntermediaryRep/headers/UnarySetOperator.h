#ifndef PDB_QUERYINTERMEDIARYREP_UNARYSETOPERATOR_H
#define PDB_QUERYINTERMEDIARYREP_UNARYSETOPERATOR_H

#include "Handle.h"
#include "SetExpressionIr.h"

using pdb::Handle;

using pdb_detail::SetExpressionIr;

namespace pdb_detail {
class UnarySetOperator {

public:
    UnarySetOperator(Handle<SetExpressionIr> operand);

    Handle<SetExpressionIr> getOperand();

private:
    Handle<SetExpressionIr> _operand;
};
}

#endif  // PDB_QUERYINTERMEDIARYREP_UNARYSETOPERATOR_H
