#include "ApplyMethod.h"

using std::invalid_argument;

namespace pdb_detail {

ApplyMethod::ApplyMethod(const string& executorId,
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
                InstructionType::apply_method) {}


void ApplyMethod::match(function<void(Load&)>,
                        function<void(ApplyFunction&)>,
                        function<void(ApplyMethod&)> forApplyMethod,
                        function<void(Filter&)>,
                        function<void(Hoist&)>,
                        function<void(GreaterThan&)>,
                        function<void(Store&)>) {
    forApplyMethod(*this);
}

ApplyMethodPtr makeApplyMethod(const string& executorId,
                               const string& functionId,
                               const string& outputTableId,
                               const string& outputColumnId,
                               TableColumns inputColumns,
                               shared_ptr<vector<TableColumn>> columnsToCopyToOutputTable) {
    try {
        return ApplyMethodPtr(new ApplyMethod(executorId,
                                              functionId,
                                              outputTableId,
                                              outputColumnId,
                                              inputColumns,
                                              columnsToCopyToOutputTable));
    } catch (const invalid_argument& e) {
        return nullptr;
    }
}
}