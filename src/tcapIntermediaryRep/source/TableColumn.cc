#include "TableColumn.h"

namespace pdb_detail {
TableColumn::TableColumn(const string& tableId, const string& columnId)
    : tableId(tableId), columnId(columnId) {}
}