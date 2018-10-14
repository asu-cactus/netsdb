#include "GreaterThanOp.h"

using std::invalid_argument;

namespace pdb_detail {

GreaterThanOp::GreaterThanOp(TcapIdentifier lhsTableName,
                             TcapIdentifier lhsColumnName,
                             TcapIdentifier rhsTableName,
                             TcapIdentifier rhsColumnName,
                             shared_ptr<RetainClause> retain)
    : lhsTableName(lhsTableName),
      lhsColumnName(lhsColumnName),
      rhsTableName(rhsTableName),
      rhsColumnName(rhsColumnName),
      retain(retain) {
    if (retain == nullptr)
        throw invalid_argument("retain may not be null");
}

void GreaterThanOp::execute(function<void(GreaterThanOp&)> forGreaterThan) {
    forGreaterThan(*this);
}
}