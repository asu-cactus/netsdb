#ifndef PDB_TCAPPARSER_RETAINCLAUSE_H
#define PDB_TCAPPARSER_RETAINCLAUSE_H

#include <functional>
#include <memory>

using std::function;
using std::shared_ptr;

namespace pdb_detail {
class RetainAllClause;  // forward declaration for match method

class RetainExplicitClause;  // forward declaration for match method

class RetainNoneClause;  // forward declaration for match method

/**
 * Base class for retention clauses of TCAP table expressions.
 *
 * A retention clause can be one of three variants extending this class:
 *
 *     RetainAllClause      - all input columns should be retained
 *     RetainNoneClause     - no input columns should be retained
 *     RetainExplicitClause - explicitly enumrated columns should be reatained
 */
class RetainClause {

public:
    /**
     * @return true if the RetainClause is an instance of RetainAllClause, else false
     */
    virtual bool isAll() = 0;

    /**
     * @return true if the RetainNoneClause is an instance of RetainAllClause, else false
     */
    virtual bool isNone() = 0;

    /**
     * If the RetainClause is an instance of RetainAllClause, invokes forAll.
     * If the RetainClause is an instance of forExplicit, invokes forExplicit.
     * If the RetainClause is an instance of RetainNoneClause, invokes forNone.
     */
    virtual void match(function<void(RetainAllClause&)> forAll,
                       function<void(RetainExplicitClause&)> forExplicit,
                       function<void(RetainNoneClause&)> forNone) = 0;
};

typedef shared_ptr<RetainClause> RetainClausePtr;
}

#endif  // PDB_TCAPPARSER_RETAINCLAUSE_H
