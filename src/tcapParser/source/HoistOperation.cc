#include "HoistOperation.h"

using std::make_shared;
using std::invalid_argument;

namespace pdb_detail {
HoistOperation::HoistOperation(string hoistTarget,
                               TcapIdentifier inputTable,
                               TcapIdentifier inputTableColumnName,
                               shared_ptr<RetainClause> retain)
    : hoistTarget(hoistTarget),
      inputTable(inputTable),
      inputTableColumnName(inputTableColumnName),
      retain(retain) {
    if (retain == nullptr)
        throw invalid_argument("retain may not be null");
}


void HoistOperation::match(function<void(LoadOperation&)>,
                           function<void(ApplyOperation&)>,
                           function<void(FilterOperation&)>,
                           function<void(HoistOperation&)> forHoist,
                           function<void(BinaryOperation&)>) {
    return forHoist(*this);
}
}