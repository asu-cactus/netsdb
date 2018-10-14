#include "ApplyOperation.h"

using std::invalid_argument;
using std::make_shared;

namespace pdb_detail {
ApplyOperation::ApplyOperation(string applyTarget,
                               ApplyOperationType applyType,
                               TcapIdentifier inputTable,
                               shared_ptr<vector<TcapIdentifier>> inputTableColumnNames,
                               shared_ptr<RetainClause> retain)
    : applyTarget(applyTarget),
      applyType(applyType),
      inputTable(inputTable),
      inputTableColumnNames(inputTableColumnNames),
      retain(retain) {
    if (inputTableColumnNames == nullptr)
        throw new std::invalid_argument("inputTableColumnNames may not be null");

    if (retain == nullptr)
        throw new std::invalid_argument("retain may not be null");
}


void ApplyOperation::match(function<void(LoadOperation&)>,
                           function<void(ApplyOperation&)> forApply,
                           function<void(FilterOperation&)>,
                           function<void(HoistOperation&)>,
                           function<void(BinaryOperation&)>) {
    forApply(*this);
}
}