#include "Filter.h"

using std::make_shared;

using std::invalid_argument;

namespace pdb_detail {

Filter::Filter(const string& inputTableId,
               const string& filterColumnId,
               const string& outputTableId,
               shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable)
    : Instruction(InstructionType::filter),
      inputTableId(inputTableId),
      filterColumnId(filterColumnId),
      outputTableId(outputTableId),
      columnsToCopyToOutputTable(columnsToCopyToOutputTable) {
    if (columnsToCopyToOutputTable == nullptr)
        throw invalid_argument("columnsToCopyToOutputTable may not be null");
}

void Filter::match(function<void(Load&)>,
                   function<void(ApplyFunction&)>,
                   function<void(ApplyMethod&)>,
                   function<void(Filter&)> forFilter,
                   function<void(Hoist&)>,
                   function<void(GreaterThan&)>,
                   function<void(Store&)>) {
    forFilter(*this);
}

FilterPtr makeFilter(const string& inputTableId,
                     const string& filterColumnId,
                     const string& outputTableId,
                     shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable) {
    try {
        return FilterPtr(
            new Filter(inputTableId, filterColumnId, outputTableId, columnsToCopyToOutputTable));
    } catch (const invalid_argument& e) {
        return nullptr;
    }
}
}