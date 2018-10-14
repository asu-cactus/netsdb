#ifndef PDB_TCAPINTERMEDIARYREP_COLUMN_H
#define PDB_TCAPINTERMEDIARYREP_COLUMN_H

#include <string>

using std::string;

namespace pdb_detail {
/**
 * Identifies a table and column.
 */
class TableColumn {
public:
    /**
     * The table.
     */
    const string tableId;

    /**
     * The column.
     */
    const string columnId;

    /**
     * Creates a new TableColumn.
     *
     * @param tableId the table
     * @param columnId the column
     * @return a new TableColumn.
     */
    TableColumn(const string& tableId, const string& columnId);
};
}

#endif  // PDB_TCAPINTERMEDIARYREP_COLUMN_H
