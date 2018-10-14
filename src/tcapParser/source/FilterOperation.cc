#include "FilterOperation.h"

using std::invalid_argument;

namespace pdb_detail {

FilterOperation::FilterOperation(TcapIdentifier inputTableName,
                                 TcapIdentifier filterColumnName,
                                 shared_ptr<RetainClause> retain)
    : inputTableName(inputTableName), filterColumnName(filterColumnName), retain(retain) {
    if (retain == nullptr)
        throw invalid_argument("retain may not be null");
}

void FilterOperation::match(function<void(LoadOperation&)>,
                            function<void(ApplyOperation&)>,
                            function<void(FilterOperation&)> forFilter,
                            function<void(HoistOperation&)>,
                            function<void(BinaryOperation&)>) {
    return forFilter(*this);
}
}