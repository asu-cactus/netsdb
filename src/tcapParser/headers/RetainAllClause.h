#ifndef PDB_TCAPPARSER_RETAINALLCLAUSE_H
#define PDB_TCAPPARSER_RETAINALLCLAUSE_H

#include <functional>

#include "RetainClause.h"

using std::function;

namespace pdb_detail {
/**
 * A retention clause that indicates that all columns from an input table should be copied into the
 * output table.
 */
class RetainAllClause : public RetainClause {
    /**
     * @return true
     */
    bool isAll() override;

    /**
     * @return false
     */
    bool isNone() override;

    // contract from super
    void match(function<void(RetainAllClause&)> forAll,
               function<void(RetainExplicitClause&)>,
               function<void(RetainNoneClause&)> forNone) override;
};
}

#endif  // PDB_TCAPPARSER_RETAINALLCLAUSE_H
