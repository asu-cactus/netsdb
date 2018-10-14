#ifndef PDB_TCAPPARSER_BINARYOPERATION_H
#define PDB_TCAPPARSER_BINARYOPERATION_H

#include <functional>

#include "TableExpression.h"

using std::function;

namespace pdb_detail {
class GreaterThanOp;

/**
 * Base class for any BinaryOperation opertion in the TCAP grammar.  For example, greater than:
 *
 *     C[examAverage] > C[hwAverage] retain student
 */
class BinaryOperation : public TableExpression {
public:
    // contract from super
    void match(function<void(LoadOperation&)>,
               function<void(ApplyOperation&)>,
               function<void(FilterOperation&)>,
               function<void(HoistOperation&)>,
               function<void(BinaryOperation&)> forBinaryOp) override;

    /**
     * Visitation hook for Visitor Pattern.
     */
    virtual void execute(function<void(GreaterThanOp&)> forGreaterThan) = 0;
};

typedef shared_ptr<BinaryOperation> BinaryOperationPtr;
}

#endif  // PDB_TCAPPARSER_BINARYOPERATION_H
