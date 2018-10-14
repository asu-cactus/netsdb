#include "RetainExplicitClause.h"

#include <vector>

using std::make_shared;
using std::vector;

using std::invalid_argument;

namespace pdb_detail {
/**
 * @param ident the ident to wrap in a vector
 * @return a vector containnig only ident
 */
shared_ptr<vector<TcapIdentifier>> wrapInVector(TcapIdentifier ident) {
    shared_ptr<vector<TcapIdentifier>> idents = make_shared<vector<TcapIdentifier>>();
    idents->push_back(ident);
    return idents;
}


RetainExplicitClause::RetainExplicitClause(shared_ptr<vector<TcapIdentifier>> columns)
    : columns(columns) {
    if (columns == nullptr)
        throw invalid_argument("columns may not be null");
}

RetainExplicitClause::RetainExplicitClause(TcapIdentifier column) : columns(wrapInVector(column)) {}

bool RetainExplicitClause::isAll() {
    return false;
}

bool RetainExplicitClause::isNone() {
    return false;
}

void RetainExplicitClause::match(function<void(RetainAllClause&)>,
                                 function<void(RetainExplicitClause&)> forExplicit,
                                 function<void(RetainNoneClause&)>) {
    forExplicit(*this);
}
}
