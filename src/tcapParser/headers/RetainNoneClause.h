#ifndef PDB_RETAINNONECLAUSE_H
#define PDB_RETAINNONECLAUSE_H

#include "RetainClause.h"

namespace pdb_detail {
/**
 * A retention clause that indicates that no columns should be copied into the output table.
 */
class RetainNoneClause : public RetainClause {
    /**
     * @return false
     */
    bool isAll();

    /**
     * @return true
     */
    bool isNone();

    // contract from super
    void match(function<void(RetainAllClause&)> forAll,
               function<void(RetainExplicitClause&)>,
               function<void(RetainNoneClause&)> forNone);
};
}

#endif  // PDB_RETAINNONECLAUSE_H
