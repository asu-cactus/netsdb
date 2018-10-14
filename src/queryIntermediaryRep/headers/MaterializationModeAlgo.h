#ifndef PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODEALGO_H
#define PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODEALGO_H

namespace pdb_detail {
class MaterializationModeNone;

class MaterializationModeNamedSet;

/**
 * Enumerates each of the variants of MaterializationMode for the Visitor Pattern.
 */
class MaterializationModeAlgo {

public:
    virtual void forNone(MaterializationModeNone& mode) = 0;

    virtual void forNamedSet(MaterializationModeNamedSet& mode) = 0;
};
}

#endif  // PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODEALGO_H
