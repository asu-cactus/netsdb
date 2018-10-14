#include "Hoist.h"

using std::make_shared;
using std::invalid_argument;

namespace pdb_detail {

Hoist::Hoist(const string& fieldId,
             TableColumn inputColumn,
             TableColumn outputColumn,
             shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable,
             const string& executorId)
    : Instruction(InstructionType::hoist),
      fieldId(fieldId),
      inputColumn(inputColumn),
      outputColumn(outputColumn),
      columnsToCopyToOutputTable(columnsToCopyToOutputTable),
      executorId(executorId) {
    if (columnsToCopyToOutputTable == nullptr)
        throw invalid_argument("columnsToCopyToOutputTable may not be null");
}

void Hoist::match(function<void(Load&)>,
                  function<void(ApplyFunction&)>,
                  function<void(ApplyMethod&)>,
                  function<void(Filter&)>,
                  function<void(Hoist&)> forHoist,
                  function<void(GreaterThan&)>,
                  function<void(Store&)>) {
    forHoist(*this);
}

HoistPtr makeHoist(const string& fieldId,
                   TableColumn inputColumn,
                   TableColumn outputColumn,
                   shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable,
                   const string& executorId) {
    try {
        return HoistPtr(
            new Hoist(fieldId, inputColumn, outputColumn, columnsToCopyToOutputTable, executorId));
    } catch (const invalid_argument& e) {
        return nullptr;
    }
}
}