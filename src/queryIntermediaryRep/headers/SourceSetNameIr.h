#ifndef PDB_QUERYINTERMEDIARYREP_SOURCESETNAMEIR_H
#define PDB_QUERYINTERMEDIARYREP_SOURCESETNAMEIR_H

#include "Handle.h"
#include "PDBString.h"
#include "SetExpressionIr.h"

using pdb::Handle;
using pdb::String;

namespace pdb_detail {
/**
 * A PDB set identified by database name and set name used as original input to a query.
 */
class SourceSetNameIr : public SetExpressionIr {

public:
    SourceSetNameIr();

    /**
     * Creates a set name from the given database name and set name.
     *
     * @param databaseName the database name.
     * @param setName the set name.
     * @return the set name.
     */
    SourceSetNameIr(string databaseName, string setName);

    // contract from super
    void execute(SetExpressionIrAlgo& algo) override;

    /**
     * @return the name of the database containing the set.
     */
    string getDatabaseName();

    /**
     * @return the name of the set.
     */
    string getSetName();

    string getName() override;


private:
    /**
     * The name of the database that contains the set.
     */
    string _databaseName;

    /**
     * The name of the set in the database.
     */
    string _setName;
};

typedef shared_ptr<SourceSetNameIr> SourceSetNameIrPt;
}

#endif  // PDB_QUERYINTERMEDIARYREP_SETNAMEIR_H
