#ifndef PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODE_H
#define PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODE_H

#include <string>

#include "MaterializationModeAlgo.h"

using std::string;

namespace pdb_detail {
/**
 * Models possible materilization options.
 *
 * An enumeration of: MaterializationModeNone, MaterializationModeNamedSet
 */
class MaterializationMode {

public:
    /**
     * @return true if materialization is not to be performed.
     */
    virtual bool isNone() = 0;

    /**
     * Visitation hook.
     */
    virtual void execute(MaterializationModeAlgo& algo) = 0;

    /**
     * @return the name of the database to materialize into, or noneValue if no materialization is
     * to be done.
     */
    virtual string tryGetDatabaseName(const string& noneValue) = 0;

    /**
     * return the name of the set to materialize into, or noneValue if no materialization is to be
     * done.
     */
    virtual string tryGetSetName(const string& noneValue) = 0;
};
}

#endif  // PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODE_H
