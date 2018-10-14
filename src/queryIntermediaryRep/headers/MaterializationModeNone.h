#ifndef PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODENONE_H
#define PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODENONE_H

#include <string>

#include "MaterializationMode.h"

using std::string;

namespace pdb_detail {
/**
* Signifies no materilization is to be done.
*/
class MaterializationModeNone : public MaterializationMode {

public:
    /**
     * @return true
     */
    bool isNone() override;

    // contract from super
    void execute(MaterializationModeAlgo& algo) override;

    // contract from super
    string tryGetDatabaseName(const string& noneValue) override;

    // contract from super
    string tryGetSetName(const string& noneValue) override;
};
}

#endif  // PDB_QUERYINTERMEDIARYREP_MATERIALIZATIONMODENONE_H
