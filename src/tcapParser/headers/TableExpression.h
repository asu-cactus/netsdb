#ifndef PDB_TCAPPARSER_TABLEEXPRESSION_H
#define PDB_TCAPPARSER_TABLEEXPRESSION_H

#include <memory>
#include <functional>

using std::function;
using std::shared_ptr;

namespace pdb_detail {
class LoadOperation;  // forward declaration for match

class ApplyOperation;  // forward declaration for match

class FilterOperation;  // forward declaration for match

class HoistOperation;  // forward declaration for match

class BinaryOperation;  // forward declaration for match

/**
 * A table value.
 */
class TableExpression {
public:
    /**
     * If the TableExpression is an instance of LoadOperation, calls forLoad.
     * If the TableExpression is an instance of ApplyOperation, calls forApply.
     * If the TableExpression is an instance of FilterOperation, calls forFilter.
     * If the TableExpression is an instance of HoistOperation, calls forHoist.
     * If the TableExpression is an instance of BinaryOperation, calls forBinaryOp.
     *
     * @param forLoad the load case
     * @param forApply the apply case
     * @param forFilter the filter case
     * @param forHoist the hoist case
     * @param forBinaryOp  the binaryOp case
     */
    virtual void match(function<void(LoadOperation&)> forLoad,
                       function<void(ApplyOperation&)> forApply,
                       function<void(FilterOperation&)> forFilter,
                       function<void(HoistOperation&)> forHoist,
                       function<void(BinaryOperation&)> forBinaryOp) = 0;
};

typedef shared_ptr<TableExpression> TableExpressionPtr;
}

#endif  // PDB_TCAPPARSER_TABLEEXPRESSION_H
