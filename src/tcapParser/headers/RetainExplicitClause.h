#ifndef PDB_TCAPPARSER_RETAINEXPLICITCLAUSE_H
#define PDB_TCAPPARSER_RETAINEXPLICITCLAUSE_H

#include <memory>
#include <vector>

#include "RetainClause.h"
#include "TcapIdentifier.h"

using std::vector;
using std::shared_ptr;


namespace pdb_detail {
/**
 * A retention clause that indicates that specific enumerated columns from an input table should be
 * copied into the output table.
 */
class RetainExplicitClause : public RetainClause {

public:
    /**
     * The columns to be retained.
     */
    const shared_ptr<const vector<TcapIdentifier>> columns;

    /**
     * Creates a RetainExplicitClause that only retains one column.
     *
     * @param column the column to retain
     * @return a new RetainExplicitClause instance.
     */
    RetainExplicitClause(TcapIdentifier column);

    /**
     * @return false
     */
    bool isAll();

    /**
     * @return false
     */
    bool isNone();

    // contract from super
    void match(function<void(RetainAllClause&)>,
               function<void(RetainExplicitClause&)> forExplicit,
               function<void(RetainNoneClause&)>);

private:
    /**
     * Creates a RetainExplicitClause that only retains one column.
     *
     * If columns is nullptr, throws invalid_argument exception
     *
     * @param column the column to retain
     * @return a new RetainExplicitClause instance.
     */
    // private because throws exception and PDB style guide forbids throwing exception across API
    // boundaries.
    RetainExplicitClause(shared_ptr<vector<TcapIdentifier>> columns);

    friend RetainClausePtr makeRetainClause(class TcapTokenStream& tokens);  // for contructor
};
}

#endif  // PDB_TCAPPARSER_RETAINEXPLICITCLAUSE_H
