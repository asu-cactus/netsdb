#include "UnarySetOperator.h"

namespace pdb_detail {

UnarySetOperator::UnarySetOperator(Handle<SetExpressionIr> operand) : _operand(operand) {}

Handle<SetExpressionIr> UnarySetOperator::getOperand() {
    return _operand;
}
}