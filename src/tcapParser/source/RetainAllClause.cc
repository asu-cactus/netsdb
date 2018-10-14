#include "RetainAllClause.h"

namespace pdb_detail {
bool RetainAllClause::isAll() {
    return true;
}

bool RetainAllClause::isNone() {
    return false;
}

void RetainAllClause::match(function<void(RetainAllClause&)> forAll,
                            function<void(RetainExplicitClause&)>,
                            function<void(RetainNoneClause&)>) {
    forAll(*this);
}
}