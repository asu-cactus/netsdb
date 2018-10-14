#ifndef PDB_TCAPINTERMEDIARYREP_COLUMN_H
#define PDB_TCAPINTERMEDIARYREP_COLUMN_H

#include <string>

using std::string;

namespace pdb_detail {
class Column {
public:
    const string tableId;

    const string columnId;

    Column(string tableId, string columnId);
};
}

#endif  // PDB_TCAPINTERMEDIARYREP_COLUMN_H
