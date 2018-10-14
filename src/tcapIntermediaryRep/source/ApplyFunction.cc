#include "ApplyFunction.h"

using std::invalid_argument;

namespace pdb_detail {

ApplyFunctionPtr makeApplyFunction(const string& executorId,
                                   const string& functionId,
                                   const string& outputTableId,
                                   const string& outputColumnId,
                                   TableColumns inputColumns,
                                   shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable) {
    try {
        return ApplyFunctionPtr(new ApplyFunction(executorId,
                                                  functionId,
                                                  outputTableId,
                                                  outputColumnId,
                                                  inputColumns,
                                                  columnsToCopyToOutputTable));
    } catch (const invalid_argument& e) {
        return nullptr;
    }
}

ApplyFunction::ApplyFunction(const string& executorId,
                             const string& functionId,
                             const string& outputTableId,
                             const string& outputColumnId,
                             TableColumns inputColumns,
                             shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable)
    : ApplyBase(executorId,
                functionId,
                outputTableId,
                outputColumnId,
                inputColumns,
                columnsToCopyToOutputTable,
                InstructionType::apply_function) {}


void ApplyFunction::match(function<void(Load&)>,
                          function<void(ApplyFunction&)> forApplyFunc,
                          function<void(ApplyMethod&)>,
                          function<void(Filter&)>,
                          function<void(Hoist&)>,
                          function<void(GreaterThan&)>,
                          function<void(Store&)> forStore) {
    forApplyFunc(*this);
}
}