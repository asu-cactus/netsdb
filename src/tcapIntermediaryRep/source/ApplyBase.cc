#include "ApplyBase.h"

using std::invalid_argument;

namespace pdb_detail {
ApplyBase::ApplyBase(const string& executorId,
                     const string& functionId,
                     const string& outputTableId,
                     const string& outputColumnId,
                     TableColumns inputColumns,
                     shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable,
                     InstructionType type)

    : Instruction(type),
      executorId(executorId),
      functionId(functionId),
      outputTableId(outputTableId),
      outputColumnId(outputColumnId),
      inputColumns(inputColumns),
      columnsToCopyToOutputTable(columnsToCopyToOutputTable) {
    if (columnsToCopyToOutputTable == nullptr)
        throw invalid_argument("columnsToCopyToOutputTable may not be null");
}

uint ApplyBase::getOutputTableColumnCount() {
    return columnsToCopyToOutputTable->size() + 1;
}
}
