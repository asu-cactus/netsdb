#include "GreaterThan.h"

using std::make_shared;
using std::invalid_argument;

namespace pdb_detail {

GreaterThan::GreaterThan(TableColumn leftHandSide,
                         TableColumn rightHandSide,
                         TableColumn outputColumn,
                         shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable,
                         const string& executorId)
    : Instruction(InstructionType::greater_than),
      leftHandSide(leftHandSide),
      rightHandSide(rightHandSide),
      outputColumn(outputColumn),
      columnsToCopyToOutputTable(columnsToCopyToOutputTable),
      executorId(executorId) {
    if (columnsToCopyToOutputTable == nullptr)
        throw invalid_argument("columnsToCopyToOutputTable may not be null");
}

void GreaterThan::match(function<void(Load&)> forLoad,
                        function<void(ApplyFunction&)>,
                        function<void(ApplyMethod&)>,
                        function<void(Filter&)>,
                        function<void(Hoist&)>,
                        function<void(GreaterThan&)> forGreaterThan,
                        function<void(Store&)>) {
    forGreaterThan(*this);
}

GreaterThanPtr makeGreaterThan(TableColumn leftHandSide,
                               TableColumn rightHandSide,
                               TableColumn outputColumn,
                               shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable,
                               const string& executorId) {
    try {
        return GreaterThanPtr(new GreaterThan(
            leftHandSide, rightHandSide, outputColumn, columnsToCopyToOutputTable, executorId));
    } catch (const invalid_argument& e) {
        return nullptr;
    }
}
}